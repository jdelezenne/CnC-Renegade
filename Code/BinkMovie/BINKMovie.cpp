/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "BinkDecoder.h"
#include "binkmovie.h"
#include "dx8wrapper.h"
#include "dx8caps.h"
#include "render2d.h"
#include "texture.h"
#include "subtitlemanager.h"
#include "wwaudio.h"
#include "Platform/Platform.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <vector>

class BINKMovieClass
{
public:
	BINKMovieClass(const char* filename, const char* subtitlename, FontCharsClass* font);
	~BINKMovieClass();
	void Update();
	void Render();
	bool Is_Complete();

private:
	void Close_Audio();
	bool Audio_Done() const;
	double Elapsed() const { return (Platform::Ticks() - StartTime) / 1000.0; }

	BinkDecoder Decoder;
	YUVbuffer Planes{};
	TextureClass* Texture = nullptr;
	Render2DClass Renderer;
	SubTitleManagerClass* Subtitles = nullptr;
	SDL_AudioStream* AudioDevice = nullptr;
	std::vector<int16_t> AudioSamples;
	unsigned AudioBufferBytes = 0;
	unsigned AudioBlockAlign = 0;
	unsigned TotalFrames = 0;
	double FrameRate = 0;
	std::uint64_t StartTime = 0;
	bool Ready = false;
	bool FrameChanged = false;
};

static BINKMovieClass* CurrentMovie = nullptr;

void BINKMovie::Play(const char* filename, const char* subtitlename, FontCharsClass* font)
{
	Stop();
	CurrentMovie = new BINKMovieClass(filename, subtitlename, font);
}

void BINKMovie::Stop()
{
	delete CurrentMovie;
	CurrentMovie = nullptr;
}

void BINKMovie::Update()
{
	if (CurrentMovie) CurrentMovie->Update();
}

void BINKMovie::Render()
{
	if (CurrentMovie) CurrentMovie->Render();
}

void BINKMovie::Init() {}
void BINKMovie::Shutdown() { Stop(); }
bool BINKMovie::Is_Complete() { return !CurrentMovie || CurrentMovie->Is_Complete(); }

BINKMovieClass::BINKMovieClass(const char* filename, const char* subtitlename, FontCharsClass* font)
{
	if (!filename || !Decoder.Open(filename)) return;
	TotalFrames = Decoder.GetNumFrames();
	FrameRate = Decoder.GetFrameRate();
	if (!TotalFrames || FrameRate <= 0 || !Decoder.frameWidth || !Decoder.frameHeight) return;

	unsigned textureWidth = 1, textureHeight = 1;
	while (textureWidth < Decoder.frameWidth) textureWidth <<= 1;
	while (textureHeight < Decoder.frameHeight) textureHeight <<= 1;
	const D3DCAPS8& caps = DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps();
	if (textureWidth > caps.MaxTextureWidth || textureHeight > caps.MaxTextureHeight) return;
	Texture = new TextureClass(textureWidth, textureHeight, WW3D_FORMAT_R5G6B5,
		TextureClass::MIP_LEVELS_1, TextureClass::POOL_MANAGED, false);
	if (!Texture->Peek_DX8_Texture()) return;
	// Clamp filtering at the movie edges instead of sampling unused texture pixels.
	Texture->Set_U_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
	Texture->Set_V_Addr_Mode(TextureClass::TEXTURE_ADDRESS_CLAMP);
	Renderer.Set_Texture(Texture);
	Renderer.Set_Coordinate_Range(RectClass(0, 0, 1, 1));
	Renderer.Add_Quad(RectClass(0, 0, 1, 1), RectClass(
		0.5f / textureWidth, 0.5f / textureHeight,
		(Decoder.frameWidth - 0.5f) / textureWidth, (Decoder.frameHeight - 0.5f) / textureHeight));

	if (Decoder.GetNumAudioTracks()) {
		const AudioInfo info = Decoder.GetAudioTrackDetails(0);
		if (info.sampleRate && (info.nChannels == 1 || info.nChannels == 2) && info.idealBufferSize) {
            SDL_AudioSpec format{};
            format.format = SDL_AUDIO_S16;
            format.channels = info.nChannels;
            format.freq = info.sampleRate;
            AudioDevice = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &format, nullptr, nullptr);
            if (AudioDevice) {
                AudioBufferBytes = info.idealBufferSize;
                AudioBlockAlign = info.nChannels * sizeof(int16_t);
                AudioSamples.resize((AudioBufferBytes + 1) / 2);
                float volume = WWAudioClass::Get_Instance() ? WWAudioClass::Get_Instance()->Get_Cinematic_Volume() : 1.0f;
                SDL_SetAudioStreamGain(AudioDevice, std::clamp(volume, 0.0f, 1.0f));
            }
		}
	}
	if (subtitlename && font) Subtitles = SubTitleManagerClass::Create(filename, subtitlename, font);
	Ready = true;
	StartTime = Platform::Ticks();
	Update(); // Decode the first frame and queue its audio before starting the clock.
	StartTime = Platform::Ticks();
	if (AudioDevice) SDL_ResumeAudioStreamDevice(AudioDevice);
}

BINKMovieClass::~BINKMovieClass()
{
	Close_Audio();
	delete Subtitles;
	REF_PTR_RELEASE(Texture);
}

void BINKMovieClass::Close_Audio()
{
    if (AudioDevice) SDL_DestroyAudioStream(AudioDevice);
    AudioDevice = nullptr;
}

bool BINKMovieClass::Audio_Done() const
{
    return !AudioDevice || (SDL_GetAudioStreamQueued(AudioDevice) == 0 &&
        SDL_GetAudioStreamAvailable(AudioDevice) == 0);
}

void BINKMovieClass::Update()
{
	if (!Ready) return;
	while (Decoder.GetCurrentFrameNum() < TotalFrames &&
		Decoder.GetCurrentFrameNum() / FrameRate <= Elapsed()) {
        // Keep the same bounded queue as the original eight-buffer player.
        if (AudioDevice && SDL_GetAudioStreamQueued(AudioDevice) >= static_cast<int>(AudioBufferBytes * 8)) break;
        Decoder.GetNextFrame(Planes);
        FrameChanged = true;
        if (AudioDevice) {
            unsigned bytes = std::min(Decoder.GetAudioData(0, AudioSamples.data()), AudioBufferBytes);
            bytes -= bytes % AudioBlockAlign;
            if (bytes && !SDL_PutAudioStreamData(AudioDevice, AudioSamples.data(), bytes)) Close_Audio();
            if (AudioDevice && Decoder.GetCurrentFrameNum() == TotalFrames) SDL_FlushAudioStream(AudioDevice);
        }
	}
}

static unsigned Clamp_Channel(int value)
{
	return static_cast<unsigned>(std::clamp(value, 0, 255));
}

void BINKMovieClass::Render()
{
	if (!Ready || !Decoder.GetCurrentFrameNum()) return;
	if (FrameChanged) {
		D3DLOCKED_RECT locked{};
		RECT rect{0, 0, static_cast<LONG>(Decoder.frameWidth), static_cast<LONG>(Decoder.frameHeight)};
		IDirect3DTexture8* texture = Texture->Peek_DX8_Texture();
		if (texture && SUCCEEDED(texture->LockRect(0, &locked, &rect, 0))) {
			for (unsigned y = 0; y < Decoder.frameHeight; ++y) {
				const uint8_t* luma = Planes[0].data + y * Planes[0].pitch;
				const uint8_t* u = Planes[1].data + (y / 2) * Planes[1].pitch;
				const uint8_t* v = Planes[2].data + (y / 2) * Planes[2].pitch;
				uint16_t* dest = reinterpret_cast<uint16_t*>(static_cast<uint8_t*>(locked.pBits) + y * locked.Pitch);
				for (unsigned x = 0; x < Decoder.frameWidth; ++x) {
					// BIKf through BIKi use limited-range BT.601 YUV420.
					const int c = 298 * (static_cast<int>(luma[x]) - 16);
					const int d = static_cast<int>(u[x / 2]) - 128;
					const int e = static_cast<int>(v[x / 2]) - 128;
					const unsigned r = Clamp_Channel((c + 409 * e + 128) >> 8);
					const unsigned g = Clamp_Channel((c - 100 * d - 208 * e + 128) >> 8);
					const unsigned b = Clamp_Channel((c + 516 * d + 128) >> 8);
					dest[x] = static_cast<uint16_t>(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
				}
			}
			texture->UnlockRect(0);
			FrameChanged = false;
		}
	}
	Renderer.Render();
	if (Subtitles) {
		Subtitles->Process(static_cast<unsigned long>(Elapsed() * 60));
		Subtitles->Render();
	}
}

bool BINKMovieClass::Is_Complete()
{
	return !Ready || (Decoder.GetCurrentFrameNum() >= TotalFrames &&
		Elapsed() >= TotalFrames / FrameRate && Audio_Done());
}
