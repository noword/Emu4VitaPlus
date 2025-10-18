#include <psp2/ctrl.h>
#include <psp2/apputil.h>
#include <psp2/system_param.h>
#include <libretro.h>
#include <stdio.h>
#include <unordered_map>
#include <string>
#include <SimpleIni.h>
#include "config.h"
#include "input.h"
#include "file.h"
#include "log.h"
#include "defines.h"
#include "utils.h"
#include "global.h"
#include "emulator.h"
#include "retro_achievements.h"

#define MAIN_SECTION "MAIN"
#define HOTKEY_SECTION "HOTKEY"
#define GRAPHICS_SECTION "GRAPHICS"
#define RA_SECTION "RETRO_ACHIEVEMENTS"

static const float SPEED_STEP[] = {0.1, 0.2, 0.5, 1.0, -1};

namespace Emu4VitaPlus
{
    std::vector<LanguageString> SPEED_STEP_OPTIONS{"0.1", "0.2", "0.5", "1.0", LANG_MAX};

#define KEY_PAIR(K) {K, #K}

    const std::unordered_map<uint32_t, const char *> Config::PsvKeyStr = {
        KEY_PAIR(SCE_CTRL_CROSS),
        KEY_PAIR(SCE_CTRL_TRIANGLE),
        KEY_PAIR(SCE_CTRL_CIRCLE),
        KEY_PAIR(SCE_CTRL_SQUARE),
        KEY_PAIR(SCE_CTRL_SELECT),
        KEY_PAIR(SCE_CTRL_START),
        KEY_PAIR(SCE_CTRL_UP),
        KEY_PAIR(SCE_CTRL_DOWN),
        KEY_PAIR(SCE_CTRL_LEFT),
        KEY_PAIR(SCE_CTRL_RIGHT),
        KEY_PAIR(SCE_CTRL_L1),
        KEY_PAIR(SCE_CTRL_R1),
        KEY_PAIR(SCE_CTRL_L2),
        KEY_PAIR(SCE_CTRL_R2),
        KEY_PAIR(SCE_CTRL_L3),
        KEY_PAIR(SCE_CTRL_R3),
        KEY_PAIR(SCE_CTRL_LSTICK_UP),
        KEY_PAIR(SCE_CTRL_LSTICK_DOWN),
        KEY_PAIR(SCE_CTRL_LSTICK_LEFT),
        KEY_PAIR(SCE_CTRL_LSTICK_RIGHT),
        KEY_PAIR(SCE_CTRL_RSTICK_UP),
        KEY_PAIR(SCE_CTRL_RSTICK_DOWN),
        KEY_PAIR(SCE_CTRL_RSTICK_LEFT),
        KEY_PAIR(SCE_CTRL_RSTICK_RIGHT),
    };

    const std::unordered_map<uint32_t, const char *> Config::HotkeyStr = {
        KEY_PAIR(SAVE_STATE),
        KEY_PAIR(LOAD_STATE),
        KEY_PAIR(EXIT_GAME),
        KEY_PAIR(GAME_SPEED_UP),
        KEY_PAIR(GAME_SPEED_DOWN),
        KEY_PAIR(GAME_REWIND),
        // KEY_PAIR(CONTROLLER_PORT_UP),
        // KEY_PAIR(CONTROLLER_PORT_DOWN),
        KEY_PAIR(SWITCH_KEYBOARD),
        KEY_PAIR(KEYBOARD_UP),
        KEY_PAIR(KEYBOARD_DOWN),
        KEY_PAIR(MENU_TOGGLE),
    };

    const std::unordered_map<uint32_t, const char *> Config::GraphicsStr = {
        KEY_PAIR(DISPLAY_SIZE),
        KEY_PAIR(DISPLAY_RATIO),
        KEY_PAIR(DISPLAY_ROTATE),
        KEY_PAIR(GRAPHICS_SHADER),
        KEY_PAIR(GRAPHICS_SMOOTH),
        KEY_PAIR(GRAPHICS_OVERLAY),
        KEY_PAIR(GRAPHICS_OVERLAY_MODE),
    };

    const std::unordered_map<uint32_t, LanguageString> Config::ControlTextMap = {
        {SCE_CTRL_CROSS, BUTTON_CROSS},
        {SCE_CTRL_TRIANGLE, BUTTON_TRIANGLE},
        {SCE_CTRL_CIRCLE, BUTTON_CIRCLE},
        {SCE_CTRL_SQUARE, BUTTON_SQUARE},
        {SCE_CTRL_SELECT, BUTTON_SELECT},
        {SCE_CTRL_START, BUTTON_START},
        {SCE_CTRL_UP, BUTTON_UP},
        {SCE_CTRL_DOWN, BUTTON_DOWN},
        {SCE_CTRL_LEFT, BUTTON_LEFT},
        {SCE_CTRL_RIGHT, BUTTON_RIGHT},
        {SCE_CTRL_L1, BUTTON_L1},
        {SCE_CTRL_R1, BUTTON_R1},
        {SCE_CTRL_L2, BUTTON_L2},
        {SCE_CTRL_R2, BUTTON_R2},
        {SCE_CTRL_L3, BUTTON_L3},
        {SCE_CTRL_R3, BUTTON_R3},
        {SCE_CTRL_LSTICK_UP, BUTTON_LEFT_ANALOG_UP},
        {SCE_CTRL_LSTICK_DOWN, BUTTON_LEFT_ANALOG_DOWN},
        {SCE_CTRL_LSTICK_LEFT, BUTTON_LEFT_ANALOG_LEFT},
        {SCE_CTRL_LSTICK_RIGHT, BUTTON_LEFT_ANALOG_RIGHT},
        {SCE_CTRL_RSTICK_UP, BUTTON_RIGHT_ANALOG_UP},
        {SCE_CTRL_RSTICK_DOWN, BUTTON_RIGHT_ANALOG_DOWN},
        {SCE_CTRL_RSTICK_LEFT, BUTTON_RIGHT_ANALOG_LEFT},
        {SCE_CTRL_RSTICK_RIGHT, BUTTON_RIGHT_ANALOG_RIGHT},
        {SCE_CTRL_PSBUTTON, BUTTON_HOME},
    };

    const std::unordered_map<uint8_t, LanguageString> Config::RetroTextMap = {
        {RETRO_DEVICE_ID_JOYPAD_B, BUTTON_B},
        {RETRO_DEVICE_ID_JOYPAD_Y, BUTTON_Y},
        {RETRO_DEVICE_ID_JOYPAD_SELECT, BUTTON_SELECT},
        {RETRO_DEVICE_ID_JOYPAD_START, BUTTON_START},
        {RETRO_DEVICE_ID_JOYPAD_UP, BUTTON_UP},
        {RETRO_DEVICE_ID_JOYPAD_DOWN, BUTTON_DOWN},
        {RETRO_DEVICE_ID_JOYPAD_LEFT, BUTTON_LEFT},
        {RETRO_DEVICE_ID_JOYPAD_RIGHT, BUTTON_RIGHT},
        {RETRO_DEVICE_ID_JOYPAD_A, BUTTON_A},
        {RETRO_DEVICE_ID_JOYPAD_X, BUTTON_X},
        {RETRO_DEVICE_ID_JOYPAD_L, BUTTON_L},
        {RETRO_DEVICE_ID_JOYPAD_R, BUTTON_R},
        {RETRO_DEVICE_ID_JOYPAD_L2, BUTTON_L2},
        {RETRO_DEVICE_ID_JOYPAD_R2, BUTTON_R2},
        {RETRO_DEVICE_ID_JOYPAD_L3, BUTTON_L3},
        {RETRO_DEVICE_ID_JOYPAD_R3, BUTTON_R3},
        {RETRO_DEVICE_ID_NONE, LANG_NONE},
    };

    Config::Config()
        : language(LANGUAGE_ENGLISH),
          hotkeys{0},
          ra_login(false),
          ra_hardcore(false)
    {
        LogFunctionName;
        Default();
    }

    Config::~Config()
    {
        LogFunctionName;
        Save();
    }

    void Config::Default()
    {
        LogFunctionName;

        version = APP_VER_STR;
        rewind = DEFAULT_ENABLE_REWIND;
        rewind_buf_size = DEFAULT_REWIND_BUF_SIZE;
        last_rom = "ux0:";
        mute = DEFAULT_MUTE;
        auto_save = DEFAULT_AUTO_SAVE;
        auto_load = DEFAULT_AUTO_LOAD;
        swap_enter = false;
        sim_button = false;
        independent_config = DEFAULT_INDEPENDENT_CONFIG;
        reboot_when_loading_again = DEFAULT_REBOOT_WHEN_LOADING_AGAIN;
        speed_step = DEFAULT_SPEED_STEP;
        support_no_game = 0;
        auto_rotating = true;
        language = Utils::GetDefaultLanguage();
        auto_download_thumbnail = (THUMBNAILS_NAME[0] != nullptr);
        ra_position = RA_POSITION_BOTTOM_RIGHT;

        DefaultControlMap();
        DefaultHotKey();
        DefaultGraphics();
        DefaultCoreOptions();
    }

    void Config::DefaultControlMap()
    {
        LogFunctionName;
        control_maps = CONTROL_MAPS;
        mouse = DEFAULT_MOUSE;
        lightgun = DEFAULT_LIGHTGUN;
        for (auto device : device_options)
        {
            device.Default();
        }
    }

    void Config::DefaultHotKey()
    {
        LogFunctionName;
        hotkeys[SAVE_STATE] = SCE_CTRL_PSBUTTON | SCE_CTRL_SQUARE;
        hotkeys[LOAD_STATE] = SCE_CTRL_PSBUTTON | SCE_CTRL_TRIANGLE;
        hotkeys[EXIT_GAME] = SCE_CTRL_PSBUTTON | SCE_CTRL_CROSS;
        hotkeys[GAME_SPEED_UP] = SCE_CTRL_PSBUTTON | SCE_CTRL_R1;
        hotkeys[GAME_SPEED_DOWN] = SCE_CTRL_PSBUTTON | SCE_CTRL_L1;
        hotkeys[GAME_REWIND] = SCE_CTRL_PSBUTTON | SCE_CTRL_RSTICK_LEFT;
        // hotkeys[CONTROLLER_PORT_UP] = SCE_CTRL_PSBUTTON | SCE_CTRL_RSTICK_UP;
        // hotkeys[CONTROLLER_PORT_DOWN] = SCE_CTRL_PSBUTTON | SCE_CTRL_RSTICK_DOWN;
        hotkeys[SWITCH_KEYBOARD] = ENABLE_KEYBOARD ? SCE_CTRL_PSBUTTON | SCE_CTRL_CIRCLE : 0;
        hotkeys[KEYBOARD_UP] = ENABLE_KEYBOARD ? SCE_CTRL_PSBUTTON | SCE_CTRL_RSTICK_UP : 0;
        hotkeys[KEYBOARD_DOWN] = ENABLE_KEYBOARD ? SCE_CTRL_PSBUTTON | SCE_CTRL_RSTICK_DOWN : 0;
        hotkeys[MENU_TOGGLE] = SCE_CTRL_PSBUTTON;
    }

    void Config::DefaultGraphics()
    {
        LogFunctionName;
        graphics[DISPLAY_SIZE] = CONFIG_DISPLAY_SIZE_FULL;
        graphics[DISPLAY_RATIO] = CONFIG_DISPLAY_RATIO_BY_GAME_RESOLUTION;
        graphics[DISPLAY_ROTATE] = CONFIG_DISPLAY_ROTATE_DEFAULT;
        graphics[GRAPHICS_SHADER] = 0;
        graphics[GRAPHICS_SMOOTH] = CONFIG_GRAPHICS_SMOOTHER_NO;
        graphics[GRAPHICS_OVERLAY_MODE] = CONFIG_GRAPHICS_OVERLAY_MODE_OVERLAY;
        graphics[GRAPHICS_OVERLAY] = 0;
    }

    void Config::DefaultCoreOptions()
    {
        LogFunctionName;
        for (const auto &setting : DEFAULT_CORE_SETTINGS)
        {
            core_options.emplace(setting.first, CoreOption{setting.second});
        }
    }

    bool Config::Save(const char *path)
    {
        LogFunctionName;

        if (path == nullptr)
        {
            path = _GetConfigFilePath();
        }

        std::string dir = File::GetDir(path);
        if (!File::Exist(dir.c_str()))
        {
            File::MakeDirs(dir.c_str());
        }

        LogDebug("save path: %s", path);

        CSimpleIniA ini;

        ini.SetValue(MAIN_SECTION, "version", version.c_str());
        ini.SetValue(MAIN_SECTION, "language", gLanguageNames[language]);
        ini.SetLongValue(MAIN_SECTION, "rewind", rewind);
        ini.SetLongValue(MAIN_SECTION, "rewind_buf_size", rewind_buf_size);
        ini.SetValue(MAIN_SECTION, "last_rom", last_rom.c_str());
        ini.SetBoolValue(MAIN_SECTION, "mute", mute);
        ini.SetBoolValue(MAIN_SECTION, "auto_save", auto_save);
        ini.SetBoolValue(MAIN_SECTION, "auto_load", auto_load);
        ini.SetLongValue(MAIN_SECTION, "mouse", mouse);
        ini.SetBoolValue(MAIN_SECTION, "lightgun", lightgun);
        ini.SetBoolValue(MAIN_SECTION, "swap_enter", swap_enter);
        ini.SetBoolValue(MAIN_SECTION, "sim_button", sim_button);
        ini.SetBoolValue(MAIN_SECTION, "independent_config", independent_config);
        ini.SetBoolValue(MAIN_SECTION, "auto_rotating", auto_rotating);
        ini.SetLongValue(MAIN_SECTION, "speed_step", speed_step);

        if (THUMBNAILS_NAME[0] != nullptr)
        {
            ini.SetBoolValue(MAIN_SECTION, "auto_download_thumbnail", auto_download_thumbnail);
        }

        if (gRetroAchievements)
        {
            ini.SetValue(RA_SECTION, "user", ra_user.c_str());
            ini.SetValue(RA_SECTION, "token", ra_token.c_str());
            ini.SetBoolValue(RA_SECTION, "login", ra_login);
            ini.SetBoolValue(RA_SECTION, "hardcore", ra_hardcore);
            ini.SetLongValue(RA_SECTION, "position", ra_position);
        }

        for (const auto &control : control_maps)
        {
            int count = 0;
            for (const auto &r : control.retros)
            {
                char tmp[16];
                snprintf(tmp, 16, "retro%02d", count++);
                ini.SetLongValue(PsvKeyStr.at(control.psv), tmp, r);
            }

            ini.SetBoolValue(PsvKeyStr.at(control.psv), "turbo", control.turbo);
            // LogDebug("%d %s", control.psv, PsvKeyStr.at(control.psv));
        }

        for (size_t i = 0; i < HOT_KEY_COUNT; i++)
        {
            ini.SetLongValue(HOTKEY_SECTION, HotkeyStr.at(i), hotkeys[i]);
        }

        for (size_t i = 0; i < GRAPHICS_CONFIG_COUNT; i++)
        {
            ini.SetLongValue(GRAPHICS_SECTION, GraphicsStr.at(i), graphics[i]);
            // LogDebug("%d %s", i, GraphicsStr.at(i));
        }

        core_options.Save(ini);
        input_descriptors.Save(ini);
        device_options.Save(ini);

        File::Remove(path);
        return ini.SaveFile(path, false) == SI_OK;
    }

    bool Config::Load(const char *path)
    {
        LogFunctionName;

        if (path == nullptr)
        {
            path = _GetConfigFilePath(true);
        }

        LogDebug("load path: %s", path);

        CSimpleIniA ini;
        if (ini.LoadFile(path) != SI_OK)
        {
            LogWarn("Failed to open %s", path);
            return false;
        }

        const char *tmp = ini.GetValue(MAIN_SECTION, "version");

        if (!(tmp && strcmp(tmp, APP_VER_STR) == 0))
        {
            float version = strtof(tmp, NULL);
            if (version < 0.31)
            {
                LogDebug("ignore config of old version");
                return false;
            }
        }

        tmp = ini.GetValue(MAIN_SECTION, "language", "ENGLISH");
        for (size_t i = 0; i < LANGUAGE_COUNT; i++)
        {
            if (strcmp(tmp, gLanguageNames[i]) == 0)
            {
                language = (LANGUAGE)i;
                break;
            }
        }

        rewind = ini.GetLongValue(MAIN_SECTION, "rewind", DEFAULT_ENABLE_REWIND);
        rewind_buf_size = ini.GetLongValue(MAIN_SECTION, "rewind_buf_size", DEFAULT_REWIND_BUF_SIZE);
        mute = ini.GetBoolValue(MAIN_SECTION, "mute", DEFAULT_MUTE);
        auto_save = ini.GetBoolValue(MAIN_SECTION, "auto_save", DEFAULT_AUTO_SAVE);
        auto_load = ini.GetBoolValue(MAIN_SECTION, "auto_load", DEFAULT_AUTO_LOAD);
        mouse = ini.GetLongValue(MAIN_SECTION, "mouse", DEFAULT_MOUSE);
        lightgun = ini.GetBoolValue(MAIN_SECTION, "lightgun", DEFAULT_LIGHTGUN);
        swap_enter = ini.GetBoolValue(MAIN_SECTION, "swap_enter", false);
        sim_button = ini.GetBoolValue(MAIN_SECTION, "sim_button", false);
        independent_config = ini.GetBoolValue(MAIN_SECTION, "independent_config", DEFAULT_INDEPENDENT_CONFIG);
        auto_rotating = ini.GetBoolValue(MAIN_SECTION, "auto_rotating", true);
        speed_step = ini.GetLongValue(MAIN_SECTION, "speed_step", 0);

        ra_user = ini.GetValue(RA_SECTION, "user", "");
        ra_token = ini.GetValue(RA_SECTION, "token", "");
        ra_login = ini.GetBoolValue(RA_SECTION, "login", false);
        ra_hardcore = ini.GetBoolValue(RA_SECTION, "hardcore", false);
        ra_position = ini.GetLongValue(RA_SECTION, "position", RA_POSITION_BOTTOM_RIGHT);

        if (THUMBNAILS_NAME[0] != nullptr)
        {
            auto_download_thumbnail = ini.GetBoolValue(MAIN_SECTION, "auto_download_thumbnail", true);
        }

        tmp = ini.GetValue(MAIN_SECTION, "last_rom");
        if (tmp && File::Exist(tmp))
        {
            last_rom = tmp;
        }
        else
        {
            last_rom = "ux0:";
        }

        rewind_buf_size = DEFAULT_REWIND_BUF_SIZE;

        for (auto &control : control_maps)
        {
            std::vector<uint8_t> retros;
            int count = 0;
            while (true)
            {
                char tmp[16];
                snprintf(tmp, 16, "retro%02d", count++);
                uint8_t r = ini.GetLongValue(PsvKeyStr.at(control.psv), tmp, RETRO_DEVICE_ID_NONE);
                if (r == RETRO_DEVICE_ID_NONE)
                    break;
                retros.push_back(r);
            }

            if (retros.size() > 0)
            {
                control.retros = retros;
                control.turbo = ini.GetBoolValue(PsvKeyStr.at(control.psv), "turbo", control.turbo);
            }
        }

        for (size_t i = 0; i < HOT_KEY_COUNT; i++)
        {
            hotkeys[i] = ini.GetLongValue(HOTKEY_SECTION, HotkeyStr.at(i), hotkeys[i]);
        }

        for (size_t i = 0; i < GRAPHICS_CONFIG_COUNT; i++)
        {
            graphics[i] = ini.GetLongValue(GRAPHICS_SECTION, GraphicsStr.at(i), graphics[i]);
        }

        core_options.Load(ini);
        input_descriptors.Load(ini);
        device_options.Load(ini);

        return true;
    }

    retro_language Config::GetRetroLanguage()
    {
        switch (language)
        {
        case LANGUAGE_CHINESE:
            return RETRO_LANGUAGE_CHINESE_SIMPLIFIED;
        case LANGUAGE_JAPANESE:
            return RETRO_LANGUAGE_JAPANESE;
        case LANGUAGE_ITALIAN:
            return RETRO_LANGUAGE_ITALIAN;
        case LANGUAGE_FRENCH:
            return RETRO_LANGUAGE_FRENCH;
        case LANGUAGE_SPANISH:
            return RETRO_LANGUAGE_SPANISH;
        case LANGUAGE_RUSSIAN:
            return RETRO_LANGUAGE_RUSSIAN;
        case LANGUAGE_ENGLISH:
        default:
            return RETRO_LANGUAGE_ENGLISH;
        }
    }

    bool Config::FrontEnabled()
    {
        return true; // for pointer
        // return mouse == CONFIG_MOUSE_FRONT || lightgun == true;
    }

    bool Config::RearEnabled()
    {
        return mouse == CONFIG_MOUSE_REAR || sim_button;
    }

    float Config::GetSpeedStep()
    {
        if (speed_step >= 0 && speed_step <= 4)
        {
            return SPEED_STEP[speed_step];
        }
        else
        {
            return 0.1;
        }
    }

    const char *Config::_GetConfigFilePath(bool must_exist)
    {
        LogFunctionName;
        static char path[SCE_FIOS_PATH_MAX];
        if (independent_config && gEmulator && *gEmulator->GetCurrentName())
        {
            snprintf(path, SCE_FIOS_PATH_MAX, "%s/%s/config.ini", CORE_SAVEFILES_DIR, gEmulator->GetCurrentName());
            if (must_exist)
            {
                return File::Exist(path) ? path : CORE_CONFIG_PATH;
            }
            else
            {
                return path;
            }
        }
        else
        {
            return CORE_CONFIG_PATH;
        }
    }
}