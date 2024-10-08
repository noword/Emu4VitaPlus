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

#if defined(ARC_BUILD)
#define DEFAULT_REWIND_BUF_SIZE 30
#else
#define DEFAULT_REWIND_BUF_SIZE 10
#endif

#define MIN_REWIND_BUF_SIZE 5
#define MAX_REWIND_BUF_SIZE 50
#define REWIND_BUF_SIZE_STEP 5

class Config
{
public:
    Config();
    virtual ~Config();

    bool Load(const char *path = APP_CONFIG_PATH);
    bool Save(const char *path = APP_CONFIG_PATH);

    void Default();
    void DefaultControlMap();
    void DefaultHotKey();
    void DefaultGraphics();
    retro_language GetRetroLanguage();

    bool boot_from_arch;
    std::string last_rom;
    uint32_t language;
    uint32_t rewind;
    uint32_t rewind_buf_size;
    std::vector<ControlMapConfig> control_maps;
    uint32_t graphics[GRAPHICS_CONFIG_COUNT];
    uint32_t hotkeys[HOT_KEY_COUNT];
    CoreOptions core_options;
    bool mute;

    static const std::unordered_map<uint32_t, const char *> PsvKeyStr;
    static const std::unordered_map<uint32_t, const char *> HotkeyStr;
    static const std::unordered_map<uint32_t, TEXT_ENUM> ControlTextMap;
    static const std::unordered_map<uint32_t, const char *> GraphicsStr;
    static const std::unordered_map<uint8_t, TEXT_ENUM> RetroTextMap;

private:
};

extern Config *gConfig;
