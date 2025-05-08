#pragma once
#include <libretro.h>

using retro_init_t = void (*)(void);
using retro_deinit_t = void (*)(void);
using retro_api_version_t = unsigned (*)(void);
using retro_get_system_info_t = void (*)(struct retro_system_info *);
using retro_get_system_av_info_t = void (*)(struct retro_system_av_info *);
using retro_set_controller_port_device_t = void (*)(unsigned, unsigned);
using retro_reset_t = void (*)(void);
using retro_run_t = void (*)(void);
using retro_serialize_size_t = size_t (*)(void);
using retro_serialize_t = bool (*)(void *, size_t);
using retro_unserialize_t = bool (*)(const void *, size_t);
using retro_cheat_reset_t = void (*)(void);
using retro_cheat_set_t = void (*)(unsigned, bool, const char *);
using retro_load_game_t = bool (*)(const struct retro_game_info *);
using retro_load_game_special_t = bool (*)(unsigned, const struct retro_game_info *, size_t);
using retro_unload_game_t = void (*)(void);
using retro_get_region_t = unsigned (*)(void);
using retro_get_memory_data_t = void *(*)(unsigned);
using retro_get_memory_size_t = size_t (*)(unsigned);
using retro_set_environment_t = void (*)(retro_environment_t);
using retro_set_video_refresh_t = void (*)(retro_video_refresh_t);
using retro_set_audio_sample_t = void (*)(retro_audio_sample_t);
using retro_set_audio_sample_batch_t = void (*)(retro_audio_sample_batch_t);
using retro_set_input_poll_t = void (*)(retro_input_poll_t);
using retro_set_input_state_t = void (*)(retro_input_state_t);

#define DECLARE_RETRO_FUNCTION(NAME) NAME##_t NAME;

struct RetroCore
{
    DECLARE_RETRO_FUNCTION(retro_init);
    DECLARE_RETRO_FUNCTION(retro_deinit);
    DECLARE_RETRO_FUNCTION(retro_api_version);
    DECLARE_RETRO_FUNCTION(retro_get_system_info);
    DECLARE_RETRO_FUNCTION(retro_get_system_av_info);
    DECLARE_RETRO_FUNCTION(retro_set_controller_port_device);
    DECLARE_RETRO_FUNCTION(retro_reset);
    DECLARE_RETRO_FUNCTION(retro_run);
    DECLARE_RETRO_FUNCTION(retro_serialize_size);
    DECLARE_RETRO_FUNCTION(retro_serialize);
    DECLARE_RETRO_FUNCTION(retro_unserialize);
    DECLARE_RETRO_FUNCTION(retro_cheat_reset);
    DECLARE_RETRO_FUNCTION(retro_cheat_set);
    DECLARE_RETRO_FUNCTION(retro_load_game);
    DECLARE_RETRO_FUNCTION(retro_load_game_special);
    DECLARE_RETRO_FUNCTION(retro_unload_game);
    DECLARE_RETRO_FUNCTION(retro_get_region);
    DECLARE_RETRO_FUNCTION(retro_get_memory_data);
    DECLARE_RETRO_FUNCTION(retro_get_memory_size);
    DECLARE_RETRO_FUNCTION(retro_set_environment);
    DECLARE_RETRO_FUNCTION(retro_set_video_refresh);
    DECLARE_RETRO_FUNCTION(retro_set_audio_sample);
    DECLARE_RETRO_FUNCTION(retro_set_audio_sample_batch);
    DECLARE_RETRO_FUNCTION(retro_set_input_poll);
    DECLARE_RETRO_FUNCTION(retro_set_input_state);
};