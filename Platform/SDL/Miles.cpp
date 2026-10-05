#define NOMINMAX
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>
#define DR_WAV_IMPLEMENTATION
#define DR_WAV_NO_STDIO
#include <dr_wav.h>
#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#include <dr_mp3.h>
#include <Mss.h>

namespace {
std::recursive_mutex AudioMutex;
ALCdevice* Device = nullptr;
ALCcontext* Context = nullptr;
SDL_AudioStream* Output = nullptr;
HDIGDRIVER Driver = nullptr;
std::atomic<int> OutputChannels{2};
int OutputRate = 44100;
bool Initialized = false;
std::array<S32, N_PREFS> Preferences{};
AIL_file_open_callback GameOpenFile = nullptr;
AIL_file_close_callback GameCloseFile = nullptr;
AIL_file_seek_callback GameSeekFile = nullptr;
AIL_file_read_callback GameReadFile = nullptr;
char Error[512]{};
HPROVIDER SelectedProvider = 1;
S32 SpeakerType = AIL_3D_2_SPEAKER;

void SetError(const char* text)
{
    SDL_strlcpy(Error, text ? text : "Audio operation failed", sizeof(Error));
    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "%s", Error);
}

struct AudioData {
    std::vector<float> PCM;
    unsigned Channels = 0;
    unsigned Rate = 0;
    unsigned Bits = 16;
    std::uint64_t Frames = 0;
};
std::unordered_map<std::uint64_t, std::weak_ptr<AudioData>> Sounds;

struct Decoder {
    drwav Wave{};
    drmp3 MP3{};
    AILFileHandle File = 0;
    bool Opened = false;
    bool IsWave = false;
    bool Valid = false;
    unsigned Channels = 0;
    unsigned Rate = 0;
    unsigned Bits = 16;
    std::uint64_t Frames = 0;

    static size_t Read(void* user, void* buffer, size_t bytes)
    {
        auto& decoder = *static_cast<Decoder*>(user);
        return GameReadFile ? GameReadFile(decoder.File, buffer, static_cast<U32>(bytes)) : 0;
    }
    static drwav_bool32 SeekWave(void* user, int offset, drwav_seek_origin origin)
    {
        auto& decoder = *static_cast<Decoder*>(user);
        U32 mode = origin == DRWAV_SEEK_SET ? AIL_FILE_SEEK_BEGIN : AIL_FILE_SEEK_CURRENT;
        return GameSeekFile && GameSeekFile(decoder.File, offset, mode) >= 0;
    }
    static drmp3_bool32 SeekMP3(void* user, int offset, drmp3_seek_origin origin)
    {
        auto& decoder = *static_cast<Decoder*>(user);
        U32 mode = origin == DRMP3_SEEK_SET ? AIL_FILE_SEEK_BEGIN : AIL_FILE_SEEK_CURRENT;
        return GameSeekFile && GameSeekFile(decoder.File, offset, mode) >= 0;
    }
    bool Load(const char* filename)
    {
        if (!GameOpenFile || !GameCloseFile || !GameSeekFile || !GameReadFile || !GameOpenFile(filename, &File)) {
            SetError("Cannot open audio through the game file factory");
            return false;
        }
        Opened = true;
        IsWave = drwav_init(&Wave, Read, SeekWave, nullptr, this, nullptr) != 0;
        if (IsWave) {
            Channels = Wave.channels; Rate = Wave.sampleRate; Bits = Wave.bitsPerSample;
            Frames = Wave.totalPCMFrameCount; Valid = true;
        } else {
            GameSeekFile(File, 0, AIL_FILE_SEEK_BEGIN);
            Valid = drmp3_init(&MP3, Read, SeekMP3, nullptr, nullptr, this, nullptr) != 0;
            if (Valid) {
                Channels = MP3.channels; Rate = MP3.sampleRate;
                Frames = drmp3_get_pcm_frame_count(&MP3);
                drmp3_seek_to_pcm_frame(&MP3, 0);
            }
        }
        if (!Valid || Channels == 0 || Channels > 2 || Rate == 0) {
            SetError("Unsupported or invalid streamed audio"); return false;
        }
        return true;
    }
    std::uint64_t ReadFrames(float* destination, std::uint64_t frames)
    {
        return IsWave ? drwav_read_pcm_frames_f32(&Wave, frames, destination)
                      : drmp3_read_pcm_frames_f32(&MP3, frames, destination);
    }
    bool Seek(std::uint64_t frame)
    {
        return IsWave ? drwav_seek_to_pcm_frame(&Wave, frame) != 0
                      : drmp3_seek_to_pcm_frame(&MP3, frame) != 0;
    }
    ~Decoder()
    {
        if (Valid) { if (IsWave) drwav_uninit(&Wave); else drmp3_uninit(&MP3); }
        if (Opened && GameCloseFile) GameCloseFile(File);
    }
};

struct Sample {
    ALuint Source = 0;
    ALuint Buffer = 0;
    ALuint Effect = 0;
    ALuint Slot = 0;
    ALuint SendFilter = 0;
    std::shared_ptr<AudioData> Data;
    std::unique_ptr<Decoder> Stream;
    std::array<AILUserData, 8> User{};
    unsigned Channels = 0;
    unsigned Rate = 0;
    unsigned Bits = 16;
    std::uint64_t Frames = 0;
    std::uint64_t Cursor = 0;
    std::uint64_t StartFrame = 0;
    double PlaybackFrame = 0;
    std::uint64_t LoopStart = 0;
    std::uint64_t LoopEnd = 0;
    U32 Loops = 1;
    U32 RemainingLoops = 1;
    U32 PlaybackLoops = 1;
    float Pitch = 1;
    S32 Volume = 127;
    S32 Pan = 64;
    S32 PlaybackRate = 0;
    bool Spatial = false;
    bool NativeSpatial = false;
    bool OwnedStream = false;
    float X = 0, Y = 0, Z = 0;
    float VX = 0, VY = 0, VZ = 0;
    float MaxDistance = 200, MinDistance = 1;
    float LeftGain = 2047.0f / 2048.0f;
    float RightGain = 2047.0f / 2048.0f;
    float EffectsLevel = 0;
    float ReflectTime = 0.01f;
    float DecayTime = 0.535f;
    HPROVIDER Processor = 0;
};
std::unordered_map<void*, std::unique_ptr<Sample>> Samples;

Sample* Find(const void* handle)
{
    auto found = Samples.find(const_cast<void*>(handle));
    return found == Samples.end() ? nullptr : found->second.get();
}

std::shared_ptr<AudioData> Decode(const void* image, size_t bytes)
{
    if (!image || bytes < 4) return {};
    const auto* source = static_cast<const unsigned char*>(image);
    std::uint64_t hash = 14695981039346656037ull;
    for (size_t i = 0; i < bytes; ++i) hash = (hash ^ source[i]) * 1099511628211ull;
    if (auto cached = Sounds[hash].lock()) return cached;
    auto data = std::make_shared<AudioData>();
    drwav_uint64 waveFrames = 0;
    float* pcm = drwav_open_memory_and_read_pcm_frames_f32(image, bytes,
        &data->Channels, &data->Rate, &waveFrames, nullptr);
    data->Frames = waveFrames;
    bool wave = pcm != nullptr;
    if (!pcm) {
        drmp3_config format{};
        drmp3_uint64 mp3Frames = 0;
        pcm = drmp3_open_memory_and_read_pcm_frames_f32(image, bytes, &format, &mp3Frames, nullptr);
        data->Frames = mp3Frames;
        data->Channels = format.channels; data->Rate = format.sampleRate;
    }
    if (!pcm) { SetError("Cannot decode audio sample"); return {}; }
    if (data->Channels == 0 || data->Channels > 2 || data->Rate == 0) {
        if (wave) drwav_free(pcm, nullptr); else drmp3_free(pcm, nullptr);
        SetError("Unsupported audio channel layout"); return {};
    }
    data->PCM.assign(pcm, pcm + data->Frames * data->Channels);
    if (wave) drwav_free(pcm, nullptr); else drmp3_free(pcm, nullptr);
    if (wave && bytes >= 36) data->Bits = source[34] | (source[35] << 8);
    Sounds[hash] = data;
    return data;
}

void Update(Sample& sample)
{
    S32 volume = std::clamp<S32>(sample.Volume, 0, 127);
    S32 pan = std::clamp<S32>(sample.Pan, 0, 127);
    float distance = std::sqrt(sample.X * sample.X + sample.Y * sample.Y + sample.Z * sample.Z);
    float pitch = sample.Rate ? static_cast<float>(sample.PlaybackRate) / sample.Rate : 1;
    if (sample.Spatial && !sample.NativeSpatial) {
        if (distance > sample.MaxDistance) volume = 0;
        else if (distance > sample.MinDistance)
            volume = static_cast<S32>(volume * sample.MinDistance / distance + 0.5f);
        float horizontal = std::hypot(sample.X, sample.Z);
        pan = horizontal > 0 ? static_cast<S32>(63.5f * (1 + sample.X / horizontal)) : 64;
    }
    if (sample.Spatial && distance > 0) {
        float radial = (sample.X * sample.VX + sample.Y * sample.VY + sample.Z * sample.VZ) / distance;
        float denominator = 0.355f + radial;
        if (std::abs(denominator) > 0.000001f) pitch *= std::abs(0.355f / denominator);
    }
    S32 full = volume == 127 ? 2047 : volume * 16;
    S32 left = pan <= 64 ? full : volume * 16 * (127 - pan) / 64;
    S32 right = pan >= 63 ? full : volume * 16 * pan / 64;
    sample.LeftGain = left / 2048.0f; sample.RightGain = right / 2048.0f;
    sample.Pitch = std::clamp(pitch, 0.001f, 16.0f);
    alSourcef(sample.Source, AL_PITCH, sample.Pitch);
    if (sample.NativeSpatial) {
        alSource3f(sample.Source, AL_POSITION, sample.X, sample.Y, -sample.Z);
        alSource3f(sample.Source, AL_VELOCITY, sample.VX * 1000, sample.VY * 1000, -sample.VZ * 1000);
        alSourcef(sample.Source, AL_REFERENCE_DISTANCE, std::max(sample.MinDistance, 0.001f));
        alSourcef(sample.Source, AL_MAX_DISTANCE, std::max(sample.MaxDistance, sample.MinDistance));
        alSourcef(sample.Source, AL_GAIN, distance > sample.MaxDistance ? 0 : full / 2048.0f);
    }
}

void UpdateEffect(Sample& sample)
{
    if (!sample.Effect) {
        alGenEffects(1, &sample.Effect); alGenAuxiliaryEffectSlots(1, &sample.Slot);
        alGenFilters(1, &sample.SendFilter);
        alFilteri(sample.SendFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    }
    alEffecti(sample.Effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
    alEffectf(sample.Effect, AL_EAXREVERB_DECAY_TIME, std::clamp(sample.DecayTime, 0.1f, 20.0f));
    alEffectf(sample.Effect, AL_EAXREVERB_REFLECTIONS_DELAY, std::clamp(sample.ReflectTime, 0.0f, 0.3f));
    alFilterf(sample.SendFilter, AL_LOWPASS_GAIN, std::clamp(sample.EffectsLevel, 0.0f, 1.0f));
    alAuxiliaryEffectSloti(sample.Slot, AL_EFFECTSLOT_EFFECT, sample.Effect);
    alSource3i(sample.Source, AL_AUXILIARY_SEND_FILTER, sample.Slot, 0, sample.SendFilter);
}

ALsizei AL_APIENTRY ReadSamples(void* user, void* destination, ALsizei bytes) noexcept
{
    auto& sample = *static_cast<Sample*>(user);
    auto* output = static_cast<float*>(destination);
    const unsigned channels = sample.NativeSpatial ? 1 : 2;
    size_t requested = bytes / (channels * sizeof(float));
    size_t written = 0;
    std::array<float, 2048> scratch{};
    while (written < requested) {
        std::uint64_t end = sample.LoopEnd ? std::min(sample.LoopEnd, sample.Frames) : sample.Frames;
        if (sample.Cursor >= end) {
            if (sample.RemainingLoops == 1 || end <= sample.LoopStart) break;
            if (sample.RemainingLoops > 1) --sample.RemainingLoops;
            sample.Cursor = sample.LoopStart;
            if (sample.Stream && !sample.Stream->Seek(sample.Cursor)) break;
        }
        size_t count = static_cast<size_t>(std::min<std::uint64_t>({ requested - written, end - sample.Cursor, 1024 }));
        if (!count) break;
        const float* input = nullptr;
        if (sample.Stream) {
            count = static_cast<size_t>(sample.Stream->ReadFrames(scratch.data(), count));
            input = scratch.data();
        } else if (sample.Data) input = sample.Data->PCM.data() + sample.Cursor * sample.Channels;
        if (!input || !count) break;
        for (size_t i = 0; i < count; ++i) {
            float left = input[i * sample.Channels];
            float right = sample.Channels > 1 ? input[i * sample.Channels + 1] : left;
            if (sample.NativeSpatial) output[written + i] = (left + right) * 0.5f;
            else {
                output[(written + i) * 2] = left * sample.LeftGain;
                output[(written + i) * 2 + 1] = right * sample.RightGain;
            }
        }
        sample.Cursor += count; written += count;
    }
    return static_cast<ALsizei>(written * channels * sizeof(float));
}

bool Bind(Sample& sample)
{
    alSourceStop(sample.Source); alSourcei(sample.Source, AL_BUFFER, 0);
    sample.Cursor = sample.StartFrame = 0;
    sample.PlaybackFrame = 0;
    sample.PlaybackRate = sample.Rate;
    sample.Loops = sample.RemainingLoops = 1;
    sample.PlaybackLoops = 1;
    sample.LoopStart = sample.LoopEnd = 0;
    alBufferCallbackSOFT(sample.Buffer, sample.NativeSpatial ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32,
        sample.Rate, ReadSamples, &sample);
    alSourcei(sample.Source, AL_BUFFER, sample.Buffer);
    Update(sample);
    if (alGetError() != AL_NO_ERROR) { SetError("OpenAL cannot attach the audio sample"); return false; }
    return true;
}

Sample* Allocate(bool spatial, bool native)
{
    if (!Context) return nullptr;
    auto sample = std::make_unique<Sample>();
    sample->Spatial = spatial; sample->NativeSpatial = native;
    alGenSources(1, &sample->Source); alGenBuffers(1, &sample->Buffer);
    if (alGetError() != AL_NO_ERROR) { SetError("OpenAL cannot allocate an audio voice"); return nullptr; }
    alSourcei(sample->Source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcei(sample->Source, AL_SOURCE_SPATIALIZE_SOFT, native ? AL_TRUE : AL_FALSE);
    if (!native) alSourcei(sample->Source, AL_DIRECT_CHANNELS_SOFT, AL_TRUE);
    auto* result = sample.get(); Samples[result] = std::move(sample);
    return result;
}

void Release(const void* handle)
{
    auto* sample = Find(handle);
    if (!sample) return;
    alSourceStop(sample->Source); alDeleteSources(1, &sample->Source); alDeleteBuffers(1, &sample->Buffer);
    if (sample->Slot) alDeleteAuxiliaryEffectSlots(1, &sample->Slot);
    if (sample->Effect) alDeleteEffects(1, &sample->Effect);
    if (sample->SendFilter) alDeleteFilters(1, &sample->SendFilter);
    Samples.erase(const_cast<void*>(handle));
}

void Render(float* pcm, int frames)
{
    for (auto& entry : Samples) {
        auto& sample = *entry.second;
        ALint state = AL_INITIAL;
        alGetSourcei(sample.Source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING || !sample.Rate || !sample.Frames) continue;
        sample.PlaybackFrame += static_cast<double>(frames) * sample.Rate * sample.Pitch / OutputRate;
        double end = sample.LoopEnd ? std::min(sample.LoopEnd, sample.Frames) : sample.Frames;
        while (sample.PlaybackFrame >= end) {
            if (sample.PlaybackLoops == 1 || end <= sample.LoopStart) { sample.PlaybackFrame = end; break; }
            if (sample.PlaybackLoops > 1) --sample.PlaybackLoops;
            sample.PlaybackFrame = sample.PlaybackFrame - end + sample.LoopStart;
        }
    }
    alcRenderSamplesSOFT(Device, pcm, frames);
}

void SDLCALL RenderAudio(void*, SDL_AudioStream* stream, int additional, int)
{
    if (additional <= 0) return;
    std::array<float, 8192> pcm{};
    std::unique_lock guard(AudioMutex, std::try_to_lock);
    int channels = OutputChannels.load();
    int frames = (additional + channels * 4 - 1) / (channels * 4);
    while (frames > 0) {
        int count = std::min(frames, 1024);
        if (guard.owns_lock() && Device) Render(pcm.data(), count);
        SDL_PutAudioStreamData(stream, pcm.data(), count * channels * sizeof(float));
        frames -= count;
    }
}

void CloseDevice()
{
    if (Output) SDL_DestroyAudioStream(Output);
    Output = nullptr;
    while (!Samples.empty()) Release(Samples.begin()->first);
    Sounds.clear();
    alcMakeContextCurrent(nullptr);
    if (Context) alcDestroyContext(Context);
    Context = nullptr;
    if (Device) alcCloseDevice(Device);
    Device = nullptr;
    delete Driver; Driver = nullptr;
}

void Start(Sample& sample)
{
    sample.Cursor = sample.StartFrame; sample.RemainingLoops = sample.Loops;
    sample.PlaybackFrame = sample.StartFrame; sample.PlaybackLoops = sample.Loops;
    if (sample.Stream) sample.Stream->Seek(sample.Cursor);
    alSourceRewind(sample.Source); alSourcePlay(sample.Source);
}

void Seek(Sample& sample, std::uint64_t frame)
{
    ALint state = AL_INITIAL; alGetSourcei(sample.Source, AL_SOURCE_STATE, &state);
    alSourceStop(sample.Source);
    sample.StartFrame = sample.Cursor = std::min(frame, sample.Frames);
    sample.PlaybackFrame = sample.StartFrame;
    if (sample.Stream) sample.Stream->Seek(sample.Cursor);
    alSourceRewind(sample.Source);
    if (state == AL_PLAYING) alSourcePlay(sample.Source);
}

S32 LengthMS(const Sample& sample)
{
    return sample.Rate ? static_cast<S32>((sample.Frames * 1000 + sample.Rate / 2) / sample.Rate) : 0;
}
S32 PositionMS(const Sample& sample)
{
    return sample.Rate ? static_cast<S32>(sample.PlaybackFrame * 1000 / sample.Rate) : 0;
}
}

extern "C" S32 AILCALL AIL_startup()
{
    std::lock_guard guard(AudioMutex);
    if (Initialized) return 1;
    Initialized = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (!Initialized) SetError(SDL_GetError());
    return Initialized;
}
extern "C" void AILCALL AIL_shutdown()
{
    std::lock_guard guard(AudioMutex);
    CloseDevice();
    if (Initialized) SDL_QuitSubSystem(SDL_INIT_AUDIO);
    Initialized = false;
}
extern "C" void AILCALL AIL_lock() { AudioMutex.lock(); }
extern "C" void AILCALL AIL_unlock() { AudioMutex.unlock(); }
extern "C" char* AILCALL AIL_last_error() { return Error; }
extern "C" S32 AILCALL AIL_set_preference(U32 number, S32 value)
{
    std::lock_guard guard(AudioMutex);
    if (number >= Preferences.size()) return -1;
    S32 previous = Preferences[number]; Preferences[number] = value; return previous;
}

extern "C" S32 AILCALL AIL_waveOutOpen(HDIGDRIVER* driver, LPHWAVEOUT*, S32, LPWAVEFORMAT format)
{
    std::lock_guard guard(AudioMutex);
    if (!driver || !format || format->nSamplesPerSec == 0 || format->nChannels < 1 || format->nChannels > 2) return 1;
    *driver = nullptr; CloseDevice();
    Device = alcLoopbackOpenDeviceSOFT(nullptr);
    if (!Device) { SetError("Cannot create OpenAL software mixer"); return 1; }
    OutputChannels = format->nChannels;
    OutputRate = format->nSamplesPerSec;
    ALCint attributes[] = { ALC_FORMAT_CHANNELS_SOFT, OutputChannels == 1 ? ALC_MONO_SOFT : ALC_STEREO_SOFT,
        ALC_FORMAT_TYPE_SOFT, ALC_FLOAT_SOFT, ALC_FREQUENCY, static_cast<ALCint>(format->nSamplesPerSec),
        ALC_MONO_SOURCES, 256, ALC_STEREO_SOURCES, 256, 0 };
    Context = alcCreateContext(Device, attributes);
    if (!Context || !alcMakeContextCurrent(Context)) { SetError("Cannot create OpenAL audio context"); CloseDevice(); return 1; }
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED); alSpeedOfSound(355.0f); alDopplerFactor(0);
    SDL_AudioSpec spec{ SDL_AUDIO_F32, OutputChannels.load(), static_cast<int>(format->nSamplesPerSec) };
    Output = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, RenderAudio, nullptr);
    if (!Output || !SDL_ResumeAudioStreamDevice(Output)) { SetError(SDL_GetError()); CloseDevice(); return 1; }
    Driver = new DIG_DRIVER{};
    Driver->emulated_ds = 0;
    *driver = Driver; Error[0] = 0; return AIL_NO_ERROR;
}
extern "C" void AILCALL AIL_waveOutClose(HDIGDRIVER driver)
{
    std::lock_guard guard(AudioMutex); if (driver == Driver) CloseDevice();
}
extern "C" HSAMPLE AILCALL AIL_allocate_sample_handle(HDIGDRIVER driver)
{
    std::lock_guard guard(AudioMutex); return driver == Driver ? reinterpret_cast<HSAMPLE>(Allocate(false, false)) : nullptr;
}
extern "C" void AILCALL AIL_release_sample_handle(HSAMPLE sample)
{
    std::lock_guard guard(AudioMutex); Release(sample);
}
extern "C" void AILCALL AIL_init_sample(HSAMPLE handle)
{
    std::lock_guard guard(AudioMutex);
    if (auto* sample = Find(handle)) {
        alSourceStop(sample->Source); alSourcei(sample->Source, AL_BUFFER, 0);
        sample->Data.reset(); sample->Stream.reset(); sample->User.fill(0);
        sample->Frames = sample->Cursor = sample->StartFrame = 0;
        sample->Volume = 127; sample->Pan = 64; sample->Processor = 0;
        sample->EffectsLevel = 0;
        if (sample->Slot) alSource3i(sample->Source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
    }
}
extern "C" S32 AILCALL AIL_set_named_sample_file(HSAMPLE handle, const C8*, const void* image, S32 bytes, S32)
{
    std::lock_guard guard(AudioMutex);
    auto* sample = Find(handle); if (!sample || bytes <= 0) return 0;
    auto data = Decode(image, bytes); if (!data) return 0;
    sample->Stream.reset(); sample->Data = std::move(data);
    sample->Channels = sample->Data->Channels; sample->Rate = sample->Data->Rate;
    sample->Bits = sample->Data->Bits; sample->Frames = sample->Data->Frames;
    return Bind(*sample);
}

#define SAMPLE_VOID(name, body) extern "C" void AILCALL name(HSAMPLE handle) { std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) { body; } }
SAMPLE_VOID(AIL_start_sample, Start(*sample))
SAMPLE_VOID(AIL_stop_sample, alSourcePause(sample->Source))
SAMPLE_VOID(AIL_resume_sample, alSourcePlay(sample->Source))
SAMPLE_VOID(AIL_end_sample, alSourceStop(sample->Source); sample->Cursor = sample->StartFrame = 0; sample->PlaybackFrame = 0; if (sample->Stream) sample->Stream->Seek(0))
#undef SAMPLE_VOID
#define SAMPLE_SET(name, field, after) extern "C" void AILCALL name(HSAMPLE handle, S32 value) { std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) { sample->field = value; after; } }
SAMPLE_SET(AIL_set_sample_volume, Volume, Update(*sample))
SAMPLE_SET(AIL_set_sample_pan, Pan, Update(*sample))
SAMPLE_SET(AIL_set_sample_playback_rate, PlaybackRate, Update(*sample))
SAMPLE_SET(AIL_set_sample_loop_count, Loops, sample->RemainingLoops = sample->PlaybackLoops = value)
#undef SAMPLE_SET
#define SAMPLE_GET(name, field) extern "C" S32 AILCALL name(HSAMPLE handle) { std::lock_guard guard(AudioMutex); auto* sample = Find(handle); return sample ? sample->field : 0; }
SAMPLE_GET(AIL_sample_volume, Volume)
SAMPLE_GET(AIL_sample_pan, Pan)
SAMPLE_GET(AIL_sample_playback_rate, PlaybackRate)
SAMPLE_GET(AIL_sample_loop_count, PlaybackLoops)
#undef SAMPLE_GET
extern "C" void AILCALL AIL_set_sample_user_data(HSAMPLE handle, U32 index, AILUserData value)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle); sample && index < sample->User.size()) sample->User[index] = value;
}
extern "C" AILUserData AILCALL AIL_sample_user_data(HSAMPLE handle, U32 index)
{
    std::lock_guard guard(AudioMutex); auto* sample = Find(handle); return sample && index < sample->User.size() ? sample->User[index] : 0;
}
extern "C" void AILCALL AIL_set_sample_ms_position(HSAMPLE handle, S32 milliseconds)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) Seek(*sample, static_cast<std::uint64_t>(std::max<S32>(milliseconds, 0)) * sample->Rate / 1000);
}
extern "C" void AILCALL AIL_sample_ms_position(HSAMPLE handle, S32* length, S32* position)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) { if (length) *length = LengthMS(*sample); if (position) *position = PositionMS(*sample); }
}
extern "C" void AILCALL AIL_set_file_callbacks(AIL_file_open_callback open, AIL_file_close_callback close, AIL_file_seek_callback seek, AIL_file_read_callback read)
{
    std::lock_guard guard(AudioMutex); GameOpenFile = open; GameCloseFile = close; GameSeekFile = seek; GameReadFile = read;
}
extern "C" HSTREAM AILCALL AIL_open_stream_by_sample(HDIGDRIVER driver, HSAMPLE handle, const char* filename, S32)
{
    std::lock_guard guard(AudioMutex);
    auto* sample = Find(handle); if (!sample || driver != Driver) return nullptr;
    auto decoder = std::make_unique<Decoder>(); if (!decoder->Load(filename)) return nullptr;
    sample->Data.reset(); sample->Stream = std::move(decoder);
    sample->Channels = sample->Stream->Channels; sample->Rate = sample->Stream->Rate;
    sample->Bits = sample->Stream->Bits; sample->Frames = sample->Stream->Frames;
    if (!Bind(*sample)) return nullptr;
    return reinterpret_cast<HSTREAM>(sample);
}
extern "C" HSTREAM AILCALL AIL_open_stream(HDIGDRIVER driver, const char* filename, S32 memory)
{
    auto handle = AIL_allocate_sample_handle(driver);
    auto stream = AIL_open_stream_by_sample(driver, handle, filename, memory);
    std::lock_guard guard(AudioMutex);
    if (auto* sample = Find(handle)) { if (stream) sample->OwnedStream = true; else Release(handle); }
    return stream;
}
extern "C" void AILCALL AIL_close_stream(HSTREAM handle)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) {
        if (sample->OwnedStream) Release(handle);
        else { alSourceStop(sample->Source); alSourcei(sample->Source, AL_BUFFER, 0); sample->Stream.reset(); }
    }
}
extern "C" void AILCALL AIL_start_stream(HSTREAM handle) { AIL_start_sample(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" void AILCALL AIL_pause_stream(HSTREAM handle, S32 pause) { if (pause) AIL_stop_sample(reinterpret_cast<HSAMPLE>(handle)); else AIL_resume_sample(reinterpret_cast<HSAMPLE>(handle)); }
#define STREAM_SET(name, sampleName) extern "C" void AILCALL name(HSTREAM handle, S32 value) { sampleName(reinterpret_cast<HSAMPLE>(handle), value); }
STREAM_SET(AIL_set_stream_volume, AIL_set_sample_volume)
STREAM_SET(AIL_set_stream_pan, AIL_set_sample_pan)
STREAM_SET(AIL_set_stream_playback_rate, AIL_set_sample_playback_rate)
STREAM_SET(AIL_set_stream_loop_count, AIL_set_sample_loop_count)
STREAM_SET(AIL_set_stream_ms_position, AIL_set_sample_ms_position)
#undef STREAM_SET
#define STREAM_GET(name, sampleName) extern "C" S32 AILCALL name(HSTREAM handle) { return sampleName(reinterpret_cast<HSAMPLE>(handle)); }
STREAM_GET(AIL_stream_volume, AIL_sample_volume)
STREAM_GET(AIL_stream_pan, AIL_sample_pan)
STREAM_GET(AIL_stream_playback_rate, AIL_sample_playback_rate)
STREAM_GET(AIL_stream_loop_count, AIL_sample_loop_count)
#undef STREAM_GET
extern "C" void AILCALL AIL_stream_ms_position(HSTREAM handle, S32* length, S32* position) { AIL_sample_ms_position(reinterpret_cast<HSAMPLE>(handle), length, position); }
extern "C" void AILCALL AIL_set_stream_loop_block(HSTREAM handle, S32 start, S32 end)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) {
        unsigned frameBytes = sample->Channels * std::max(sample->Bits / 8, 1u);
        sample->LoopStart = std::max<S32>(start, 0) / frameBytes;
        sample->LoopEnd = end < 0 ? sample->Frames : end / frameBytes;
    }
}

extern "C" S32 AILCALL AIL_WAV_info(const void* image, AILSOUNDINFO* info)
{
    if (!image || !info) return 0;
    const auto* bytes = static_cast<const unsigned char*>(image);
    if (std::memcmp(bytes, "RIFF", 4) || std::memcmp(bytes + 8, "WAVE", 4)) return 0;
    auto word = [](const unsigned char* p) { return static_cast<U32>(p[0]) | static_cast<U32>(p[1]) << 8; };
    auto integer = [](const unsigned char* p) { return static_cast<U32>(p[0]) | static_cast<U32>(p[1]) << 8 | static_cast<U32>(p[2]) << 16 | static_cast<U32>(p[3]) << 24; };
    U32 size = integer(bytes + 4) + 8;
    *info = {};
    for (U32 offset = 12; offset <= size - 8 && offset < AIL_MAX_FILE_HEADER_SIZE - 24;) {
        U32 length = integer(bytes + offset + 4);
        if (!std::memcmp(bytes + offset, "fmt ", 4) && length >= 16) {
            info->format = word(bytes + offset + 8); info->channels = word(bytes + offset + 10);
            info->rate = integer(bytes + offset + 12); info->bits = word(bytes + offset + 22);
            info->block_size = word(bytes + offset + 20);
        } else if (!std::memcmp(bytes + offset, "data", 4)) {
            info->data_ptr = bytes + offset + 8; info->initial_ptr = image; info->data_len = length;
            info->samples = info->bits && info->channels ? length * 8 / (info->bits * info->channels) : 0;
            return info->rate && info->channels;
        }
        if (length > size - offset - 8) break;
        offset += 8 + length + (length & 1);
    }
    return 0;
}

extern "C" S32 AILCALL AIL_enumerate_3D_providers(HPROENUM* next, HPROVIDER* provider, C8** name)
{
    static char fast[] = "SDL3 Fast 2D Positional Audio";
    static char spatial[] = "OpenAL Soft 3D Audio";
    static char eax[] = "OpenAL Soft EAX Reverb";
    static char* names[] = { fast, spatial, eax };
    if (!next || !provider || !name || *next >= 3) return 0;
    *provider = *next + 1; *name = names[*next]; ++*next; return 1;
}
extern "C" M3DRESULT AILCALL AIL_open_3D_provider(HPROVIDER provider)
{
    std::lock_guard guard(AudioMutex);
    if (!Context || provider < 1 || provider > 3) return M3D_NOT_INIT;
    SelectedProvider = provider; return M3D_NOERR;
}
extern "C" void AILCALL AIL_close_3D_provider(HPROVIDER provider)
{
    std::lock_guard guard(AudioMutex); if (SelectedProvider == provider) SelectedProvider = 1;
}
extern "C" H3DSAMPLE AILCALL AIL_allocate_3D_sample_handle(HPROVIDER provider)
{
    std::lock_guard guard(AudioMutex); return reinterpret_cast<H3DSAMPLE>(Allocate(true, provider != 1));
}
extern "C" void AILCALL AIL_release_3D_sample_handle(H3DSAMPLE handle) { AIL_release_sample_handle(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" void AILCALL AIL_start_3D_sample(H3DSAMPLE handle) { AIL_start_sample(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" void AILCALL AIL_stop_3D_sample(H3DSAMPLE handle) { AIL_stop_sample(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" void AILCALL AIL_resume_3D_sample(H3DSAMPLE handle) { AIL_resume_sample(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" void AILCALL AIL_end_3D_sample(H3DSAMPLE handle) { AIL_end_sample(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" S32 AILCALL AIL_set_3D_sample_file(H3DSAMPLE handle, const void* image)
{
    if (!image) return 0;
    U32 length = 0; std::memcpy(&length, static_cast<const unsigned char*>(image) + 4, 4);
    return AIL_set_named_sample_file(reinterpret_cast<HSAMPLE>(handle), ".wav", image, length + 8, 0);
}
extern "C" void AILCALL AIL_set_3D_sample_volume(H3DSAMPLE handle, S32 value) { AIL_set_sample_volume(reinterpret_cast<HSAMPLE>(handle), value); }
extern "C" void AILCALL AIL_set_3D_sample_playback_rate(H3DSAMPLE handle, S32 value) { AIL_set_sample_playback_rate(reinterpret_cast<HSAMPLE>(handle), value); }
extern "C" void AILCALL AIL_set_3D_sample_loop_count(H3DSAMPLE handle, U32 value) { AIL_set_sample_loop_count(reinterpret_cast<HSAMPLE>(handle), value); }
extern "C" S32 AILCALL AIL_3D_sample_volume(H3DSAMPLE handle) { return AIL_sample_volume(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" S32 AILCALL AIL_3D_sample_playback_rate(H3DSAMPLE handle) { return AIL_sample_playback_rate(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" U32 AILCALL AIL_3D_sample_loop_count(H3DSAMPLE handle) { return AIL_sample_loop_count(reinterpret_cast<HSAMPLE>(handle)); }
extern "C" U32 AILCALL AIL_3D_sample_length(H3DSAMPLE handle)
{
    std::lock_guard guard(AudioMutex); auto* sample = Find(handle); return sample ? static_cast<U32>(sample->Frames * sample->Channels * sample->Bits / 8) : 0;
}
extern "C" U32 AILCALL AIL_3D_sample_offset(H3DSAMPLE handle)
{
    std::lock_guard guard(AudioMutex); auto* sample = Find(handle); return sample ? static_cast<U32>(sample->PlaybackFrame * sample->Channels * sample->Bits / 8) : 0;
}
extern "C" void AILCALL AIL_set_3D_sample_offset(H3DSAMPLE handle, U32 offset)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) Seek(*sample, offset / std::max(sample->Channels * sample->Bits / 8, 1u));
}
extern "C" void AILCALL AIL_set_3D_sample_distances(H3DSAMPLE handle, F32 maximum, F32 minimum)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) { sample->MaxDistance = maximum; sample->MinDistance = minimum; Update(*sample); }
}
extern "C" void AILCALL AIL_set_3D_object_user_data(H3DPOBJECT handle, U32 index, AILUserData value) { AIL_set_sample_user_data(reinterpret_cast<HSAMPLE>(handle), index, value); }
extern "C" AILUserData AILCALL AIL_3D_object_user_data(H3DPOBJECT handle, U32 index) { return AIL_sample_user_data(reinterpret_cast<HSAMPLE>(handle), index); }
extern "C" void AILCALL AIL_set_3D_position(H3DPOBJECT handle, F32 x, F32 y, F32 z)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) { sample->X = x; sample->Y = y; sample->Z = z; Update(*sample); }
}
extern "C" void AILCALL AIL_set_3D_velocity_vector(H3DPOBJECT handle, F32 x, F32 y, F32 z)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) { sample->VX = x; sample->VY = y; sample->VZ = z; Update(*sample); }
}
extern "C" void AILCALL AIL_set_3D_orientation(H3DPOBJECT handle, F32 x, F32 y, F32 z, F32, F32, F32)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) alSource3f(sample->Source, AL_DIRECTION, x, y, -z);
}
extern "C" void AILCALL AIL_set_3D_speaker_type(HPROVIDER, S32 type)
{
    std::lock_guard guard(AudioMutex);
    if (!Device || !Output) return;
    int channels = OutputChannels == 1 ? 1 : 2;
    ALCenum layout = channels == 1 ? ALC_MONO_SOFT : ALC_STEREO_SOFT;
    if (type == AIL_3D_SURROUND) { channels = 6; layout = ALC_5POINT1_SOFT; }
    else if (type == AIL_3D_4_SPEAKER) { channels = 4; layout = ALC_QUAD_SOFT; }
    ALCint attributes[] = { ALC_FORMAT_CHANNELS_SOFT, layout, ALC_FORMAT_TYPE_SOFT, ALC_FLOAT_SOFT,
        ALC_FREQUENCY, OutputRate, ALC_HRTF_SOFT, type == AIL_3D_HEADPHONE ? ALC_TRUE : ALC_FALSE, 0 };
    if (!alcResetDeviceSOFT(Device, attributes)) { SetError("Cannot configure the audio speaker layout"); return; }
    SDL_AudioSpec spec{ SDL_AUDIO_F32, channels, OutputRate };
    if (!SDL_SetAudioStreamFormat(Output, &spec, nullptr)) { SetError(SDL_GetError()); return; }
    OutputChannels = channels; SpeakerType = type;
}
extern "C" void AILCALL AIL_set_3D_sample_effects_level(H3DSAMPLE handle, F32 level)
{
    std::lock_guard guard(AudioMutex); if (auto* sample = Find(handle)) { sample->EffectsLevel = level; UpdateEffect(*sample); }
}
extern "C" H3DPOBJECT AILCALL AIL_open_3D_listener(HPROVIDER)
{
    std::lock_guard guard(AudioMutex); return reinterpret_cast<H3DPOBJECT>(Allocate(false, false));
}
extern "C" S32 AILCALL AIL_enumerate_filters(HPROENUM* next, HPROVIDER* provider, C8** name)
{
    static char filter[] = "OpenAL Soft Radio Reverb";
    if (!Context || !next || !provider || !name || *next != HPROENUM_FIRST) return 0;
    *next = 1; *provider = 4; *name = filter; return 1;
}
extern "C" HPROVIDER AILCALL AIL_set_sample_processor(HSAMPLE handle, SAMPLESTAGE stage, HPROVIDER provider)
{
    std::lock_guard guard(AudioMutex); auto* sample = Find(handle); if (!sample || stage != DP_FILTER) return 0;
    HPROVIDER old = sample->Processor; sample->Processor = provider;
    sample->EffectsLevel = provider ? 0.3f : 0; UpdateEffect(*sample); return old;
}
extern "C" void AILCALL AIL_set_filter_sample_preference(HSAMPLE handle, const C8* name, const void* value)
{
    std::lock_guard guard(AudioMutex); auto* sample = Find(handle); if (!sample || !name || !value) return;
    float setting = *static_cast<const float*>(value);
    if (!std::strcmp(name, "Reverb level")) sample->EffectsLevel = setting;
    else if (!std::strcmp(name, "Reverb reflect time")) sample->ReflectTime = setting;
    else if (!std::strcmp(name, "Reverb decay time")) sample->DecayTime = setting;
    else { SetError("Unknown audio filter preference"); return; }
    UpdateEffect(*sample);
}
extern "C" void AILCALL AIL_stop_timer(HTIMER) { SetError("The game does not allocate Miles audio timers"); }
extern "C" void AILCALL AIL_release_timer_handle(HTIMER) { SetError("The game does not allocate Miles audio timers"); }
