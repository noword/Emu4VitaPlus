#pragma once
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string>
#include "libretro.h"
#include "core_options.h"
#include "config_types.h"
#include "language_define.h"
#include "defines.h"
#include "input_descriptor.h"
#include "device_options.h"

#define MIN_REWIND_BUF_SIZE 5
#define MAX_REWIND_BUF_SIZE 50
#define REWIND_BUF_SIZE_STEP 5
#define DEFAULT_MUTE false

namespace Emu4VitaPlus
{
    extern std::vector<LanguageString> SPEED_STEP_OPTIONS;

    class Config
    {
    public:
        Config();
        virtual ~Config();

        bool Load(const char *path = nullptr);
        bool Save(const char *path = nullptr);

        void Default();
        void DefaultControlMap();
        void DefaultHotKey();
        void DefaultGraphics();
        void DefaultCoreOptions();

        retro_language GetRetroLanguage();
        bool FrontEnabled();
        bool RearEnabled();
        float GetSpeedStep();

        std::string version;
        bool reboot_when_loading_again;
        std::string last_rom;
        uint32_t language;
        uint32_t rewind;
        uint32_t rewind_buf_size;
        std::vector<ControlMapConfig> control_maps;
        uint32_t graphics[GRAPHICS_CONFIG_COUNT];
        uint32_t hotkeys[HOT_KEY_COUNT];
        CoreOptions core_options;
        InputDescriptors input_descriptors;
        DeviceOptions device_options;
        uint32_t mute;
        uint32_t auto_save;
        uint32_t auto_load;
        uint32_t mouse;
        uint32_t lightgun;
        uint32_t swap_enter;
        uint32_t sim_button;
        uint32_t independent_config;
        uint32_t speed_step;
        uint32_t support_no_game;
        uint32_t auto_rotating;
        uint32_t auto_download_thumbnail;
        uint32_t cpu_freq;
        uint32_t fps;

        std::string ra_user;
        std::string ra_password;
        std::string ra_token;
        uint32_t ra_hardcore;
        uint32_t ra_login;
        uint32_t ra_position;

        static const std::unordered_map<uint32_t, const char *> PsvKeyStr;
        static const std::unordered_map<uint32_t, const char *> HotkeyStr;
        static const std::unordered_map<uint32_t, LanguageString> ControlTextMap;
        static const std::unordered_map<uint32_t, const char *> GraphicsStr;
        static const std::unordered_map<uint8_t, LanguageString> RetroTextMap;

    private:
        const char *_GetConfigFilePath(bool must_exist = false);
    };
}
