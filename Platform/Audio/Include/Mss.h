#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#ifndef NO
#define NO 0
#endif
#ifndef YES
#define YES 1
#endif

using S32 = long;
using U32 = unsigned long;
using F32 = float;
using C8 = char;
struct MilesSample;
struct MilesStream;
struct Miles3DObject;
using HSAMPLE = MilesSample*;
using HSTREAM = MilesStream*;
using H3DPOBJECT = Miles3DObject*;
using H3DSAMPLE = H3DPOBJECT;
struct DIG_DRIVER { S32 emulated_ds; };
using HDIGDRIVER = DIG_DRIVER*;
using HPROVIDER = U32;
using HPROENUM = U32;
using HTIMER = S32;
using M3DRESULT = S32;
#define AILCALL WINAPI
#define AILCALLBACK WINAPI
#define AIL_set_3D_object_user_data AIL_set_3D_user_data
#define AIL_3D_object_user_data AIL_3D_user_data
#define AIL_3D_open_listener AIL_open_3D_listener
inline constexpr U32 N_PREFS = 46;
inline constexpr U32 DIG_USE_WAVEOUT = 15;
inline constexpr U32 AIL_LOCK_PROTECTION = 18;
inline constexpr U32 AIL_MAX_FILE_HEADER_SIZE = 4096;
inline constexpr S32 AIL_NO_ERROR = 0;
inline constexpr S32 M3D_NOERR = 0;
inline constexpr S32 M3D_NOT_INIT = 8;
inline constexpr HPROENUM HPROENUM_FIRST = 0;
inline constexpr U32 AIL_FILE_SEEK_BEGIN = 0;
inline constexpr U32 AIL_FILE_SEEK_CURRENT = 1;
inline constexpr U32 AIL_FILE_SEEK_END = 2;
inline constexpr S32 AIL_3D_2_SPEAKER = 0;
inline constexpr S32 AIL_3D_HEADPHONE = 1;
inline constexpr S32 AIL_3D_SURROUND = 2;
inline constexpr S32 AIL_3D_4_SPEAKER = 3;
inline constexpr S32 ENVIRONMENT_GENERIC = 0;
enum SAMPLESTAGE { DP_ASI_DECODER, DP_FILTER, DP_MERGE, N_SAMPLE_STAGES, SAMPLE_ALL_STAGES };
struct AILSOUNDINFO {
 S32 format;
 const void* data_ptr;
 U32 data_len;
 U32 rate;
 S32 bits;
 S32 channels;
 U32 samples;
 U32 block_size;
 const void* initial_ptr;
};
using AIL_file_open_callback = U32 (AILCALLBACK*)(const char*, U32*);
using AIL_file_close_callback = void (AILCALLBACK*)(U32);
using AIL_file_seek_callback = S32 (AILCALLBACK*)(U32, S32, U32);
using AIL_file_read_callback = U32 (AILCALLBACK*)(U32, void*, U32);

extern "C" {
S32 AILCALL AIL_startup (void);
void AILCALL AIL_shutdown (void);
S32 AILCALL AIL_set_preference (U32 number, S32 value);
char *AILCALL AIL_last_error (void);
void AILCALL AIL_lock (void);
void AILCALL AIL_unlock (void);
void AILCALL AIL_stop_timer (HTIMER timer);
void AILCALL AIL_release_timer_handle (HTIMER timer);
S32 AILCALL AIL_waveOutOpen (HDIGDRIVER *drvr, LPHWAVEOUT *lphWaveOut, S32 wDeviceID, LPWAVEFORMAT lpFormat);
void AILCALL AIL_waveOutClose (HDIGDRIVER drvr);
HSAMPLE AILCALL AIL_allocate_sample_handle (HDIGDRIVER dig);
void AILCALL AIL_release_sample_handle (HSAMPLE S);
void AILCALL AIL_init_sample (HSAMPLE S);
S32 AILCALL AIL_set_named_sample_file (HSAMPLE S, C8 const *file_type_suffix, void const *file_image, S32 file_size, S32 block);
HPROVIDER AILCALL AIL_set_sample_processor (HSAMPLE S, SAMPLESTAGE pipeline_stage, HPROVIDER provider);
void AILCALL AIL_start_sample (HSAMPLE S);
void AILCALL AIL_stop_sample (HSAMPLE S);
void AILCALL AIL_resume_sample (HSAMPLE S);
void AILCALL AIL_end_sample (HSAMPLE S);
void AILCALL AIL_set_sample_playback_rate (HSAMPLE S, S32 playback_rate);
void AILCALL AIL_set_sample_volume (HSAMPLE S, S32 volume);
void AILCALL AIL_set_sample_pan (HSAMPLE S, S32 pan);
void AILCALL AIL_set_sample_loop_count (HSAMPLE S, S32 loop_count);
S32 AILCALL AIL_sample_playback_rate (HSAMPLE S);
S32 AILCALL AIL_sample_volume (HSAMPLE S);
S32 AILCALL AIL_sample_pan (HSAMPLE S);
S32 AILCALL AIL_sample_loop_count (HSAMPLE S);
void AILCALL AIL_set_sample_user_data (HSAMPLE S, U32 index, S32 value);
S32 AILCALL AIL_sample_user_data (HSAMPLE S, U32 index);
void AILCALL AIL_set_sample_ms_position (HSAMPLE S, S32 milliseconds);
void AILCALL AIL_sample_ms_position (HSAMPLE S, S32 * total_milliseconds, S32 * current_milliseconds);
HSTREAM AILCALL AIL_open_stream(HDIGDRIVER dig, char const * filename, S32 stream_mem);
void AILCALL AIL_close_stream(HSTREAM stream);
void AILCALL AIL_start_stream(HSTREAM stream);
void AILCALL AIL_pause_stream(HSTREAM stream, S32 onoff);
void AILCALL AIL_set_stream_volume(HSTREAM stream,S32 volume);
void AILCALL AIL_set_stream_pan(HSTREAM stream,S32 pan);
S32 AILCALL AIL_stream_volume(HSTREAM stream);
S32 AILCALL AIL_stream_pan(HSTREAM stream);
void AILCALL AIL_set_stream_playback_rate(HSTREAM stream, S32 rate);
S32 AILCALL AIL_stream_playback_rate(HSTREAM stream);
S32 AILCALL AIL_stream_loop_count(HSTREAM stream);
void AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count);
void AILCALL AIL_set_stream_loop_block (HSTREAM S, S32 loop_start_offset, S32 loop_end_offset);
void AILCALL AIL_set_stream_ms_position (HSTREAM S, S32 milliseconds);
void AILCALL AIL_stream_ms_position (HSTREAM S, S32 * total_milliseconds, S32 * current_milliseconds);
void AILCALL AIL_set_file_callbacks (AIL_file_open_callback opencb, AIL_file_close_callback closecb, AIL_file_seek_callback seekcb, AIL_file_read_callback readcb);
S32 AILCALL AIL_WAV_info(void const * data, AILSOUNDINFO * info);
S32 AILCALL AIL_enumerate_filters (HPROENUM *next, HPROVIDER *dest, C8 * *name);
void AILCALL AIL_set_filter_sample_preference (HSAMPLE S, C8 const * name, void const * val);
S32 AILCALL AIL_enumerate_3D_providers (HPROENUM *next, HPROVIDER *dest, C8 * *name);
M3DRESULT AILCALL AIL_open_3D_provider (HPROVIDER lib);
void AILCALL AIL_close_3D_provider (HPROVIDER lib);
H3DSAMPLE AILCALL AIL_allocate_3D_sample_handle (HPROVIDER lib);
void AILCALL AIL_release_3D_sample_handle (H3DSAMPLE S);
void AILCALL AIL_start_3D_sample (H3DSAMPLE S);
void AILCALL AIL_stop_3D_sample (H3DSAMPLE S);
void AILCALL AIL_resume_3D_sample (H3DSAMPLE S);
void AILCALL AIL_end_3D_sample (H3DSAMPLE S);
S32 AILCALL AIL_set_3D_sample_file (H3DSAMPLE S, void const *file_image);
void AILCALL AIL_set_3D_sample_volume (H3DSAMPLE S, S32 volume);
void AILCALL AIL_set_3D_sample_offset (H3DSAMPLE S, U32 offset);
void AILCALL AIL_set_3D_sample_playback_rate (H3DSAMPLE S, S32 playback_rate);
void AILCALL AIL_set_3D_sample_loop_count(H3DSAMPLE S, U32 loops);
S32 AILCALL AIL_3D_sample_volume (H3DSAMPLE S);
U32 AILCALL AIL_3D_sample_offset (H3DSAMPLE S);
S32 AILCALL AIL_3D_sample_playback_rate (H3DSAMPLE S);
U32 AILCALL AIL_3D_sample_length (H3DSAMPLE S);
U32 AILCALL AIL_3D_sample_loop_count (H3DSAMPLE S);
void AILCALL AIL_set_3D_speaker_type (HPROVIDER lib, S32 speaker_type);
void AILCALL AIL_set_3D_sample_distances (H3DSAMPLE S, F32 max_dist, F32 min_dist);
void AILCALL AIL_set_3D_sample_effects_level (H3DSAMPLE S, F32 effects_level);
H3DPOBJECT AILCALL AIL_open_3D_listener (HPROVIDER lib);
void AILCALL AIL_set_3D_position (H3DPOBJECT obj, F32 X, F32 Y, F32 Z);
void AILCALL AIL_set_3D_velocity_vector (H3DPOBJECT obj, F32 dX_per_ms, F32 dY_per_ms, F32 dZ_per_ms);
void AILCALL AIL_set_3D_orientation (H3DPOBJECT obj, F32 X_face, F32 Y_face, F32 Z_face, F32 X_up, F32 Y_up, F32 Z_up);
void AILCALL AIL_set_3D_user_data (H3DPOBJECT obj, U32 index, S32 value);
S32 AILCALL AIL_3D_user_data (H3DPOBJECT obj, U32 index);
HSTREAM AILCALL AIL_open_stream_by_sample(HDIGDRIVER driver, HSAMPLE sample, const char *filename, S32 stream_mem);
}
