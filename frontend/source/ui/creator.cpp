#include "global.h"
#include "ui.h"
#include "state_manager.h"
#include "tab_selectable.h"
#include "tab_control.h"
#include "item_config.h"
#include "item_control.h"
#include "item_hotkey.h"
#include "item_core.h"
#include "item_state.h"
#include "item_device.h"
#include "item_achievement.h"
#include "item_theme.h"
#include "tab_browser.h"
#include "tab_favorite.h"
#include "tab_about.h"
#include "tab_disk.h"
#include "utils.h"
#include "overlay.h"
#include "shader.h"
#include "defines.h"
#include "misc.h"

void Ui::CreateTables()
{
    LogFunctionName;

    if (gEmulator == nullptr)
    {
        LogError("gEmulator has not been initialized yet.");
        return;
    }

    _ClearTabs();

    std::vector<ItemBase *> items{
        new ItemBase(LANG_START_CORE_NO_GAME, "", std::bind(&Ui::_OnStartCore, this, &_input)),
        new ItemBase(LANG_RESUME_GAME, "", std::bind(&Ui::_ResumeGame, this), NULL, false),
        new ItemBase(LANG_RESET_GAME, "", std::bind(&Ui::_ResetGame, this), NULL, false),
        new ItemBase(LANG_EXIT_GAME, "", std::bind(&Ui::_ExitGame, this), NULL, false),
        new ItemBase(LANG_BACK_TO_ARCH, "", std::bind(&Ui::_ReturnToArch, this)),
        new ItemBase(LANG_CLEAN_CACHE, "", std::bind(&Ui::_OnCleanCache, this, &_input)),
        new ItemBase(LANG_EXIT, "", std::bind(&Ui::_ExitApp, this))};

    items[0]->SetVisable(gConfig->support_no_game);
    items[4]->SetVisable(gBootFromArch);

    _tabs[TAB_INDEX_SYSTEM] = new TabSeletable(LANG_SYSTEM, items);

    std::vector<ItemBase *> states;
    states.reserve(MAX_STATES);
    for (size_t i = 0; i < MAX_STATES; i++)
    {
        states.emplace_back(new ItemState(gStateManager->states[i]));
    }
    _tabs[TAB_INDEX_STATE] = new TabSeletable(LANG_STATE, states, 1);
    _tabs[TAB_INDEX_STATE]->SetVisable(false);

    _tabs[TAB_INDEX_CHEAT] = new TabSeletable(LANG_CHEAT);
    _tabs[TAB_INDEX_CHEAT]->SetVisable(false);

    _tabs[TAB_INDEX_ACHIEVEMENTS] = new TabSeletable(LANG_ACHIEVEMENTS, 1);
    _tabs[TAB_INDEX_ACHIEVEMENTS]->SetVisable(false);

    _tabs[TAB_INDEX_BROWSER] = new TabBrowser();
    _tabs[TAB_INDEX_FAVORITE] = new TabFavorite();

    _tabs[TAB_INDEX_GRAPHICS] = new TabSeletable(LANG_GRAPHICS,
                                                 {new ItemConfig(LANG_DISPLAY_SIZE,
                                                                 "",
                                                                 &gConfig->graphics[DISPLAY_SIZE],
                                                                 {LANG_1X, LANG_2X, LANG_3X, LANG_FULL},
                                                                 std::bind(&Emulator::ChangeGraphicsConfig, gEmulator)),
                                                  new ItemConfig(LANG_ASPECT_RATIO,
                                                                 "",
                                                                 &gConfig->graphics[DISPLAY_RATIO],
                                                                 {LANG_BY_GAME_RESOLUTION,
                                                                  LANG_BY_DEV_SCREEN,
                                                                  LANG_8_7,
                                                                  LANG_4_3,
                                                                  LANG_3_2,
                                                                  LANG_16_9},
                                                                 std::bind(&Emulator::ChangeGraphicsConfig, gEmulator)),
#ifdef WANT_DISPLAY_ROTATE
                                                  new ItemConfig(LANG_DISPLAY_ROTATE,
                                                                 "",
                                                                 &gConfig->graphics[DISPLAY_ROTATE],
                                                                 DISPLAY_ROTATE_DISABLE,
                                                                 CONFIG_DISPLAY_ROTATE_COUNT),
#endif
                                                  new ItemConfig(LANG_GRAPHICS_SHADER,
                                                                 "",
                                                                 &gConfig->graphics[GRAPHICS_SHADER],
                                                                 gShaders->GetConfigs(),
                                                                 std::bind(&Emulator::ChangeGraphicsConfig, gEmulator)),
                                                  new ItemConfig(LANG_GRAPHICS_SMOOTH,
                                                                 "",
                                                                 &gConfig->graphics[GRAPHICS_SMOOTH],
                                                                 {LANG_NO, LANG_YES},
                                                                 std::bind(&Emulator::ChangeGraphicsConfig, gEmulator)),
                                                  new ItemConfig(LANG_OVERLAY_MODE,
                                                                 "",
                                                                 &gConfig->graphics[GRAPHICS_OVERLAY_MODE],
                                                                 {LANG_MODE_OVERLAY, LANG_MODE_BACKGROUND},
                                                                 std::bind(&Emulator::ChangeGraphicsConfig, gEmulator)),
                                                  new ItemConfig(LANG_OVERLAY_SELECT,
                                                                 "",
                                                                 &gConfig->graphics[GRAPHICS_OVERLAY],
                                                                 gOverlays->GetConfigs(),
                                                                 std::bind(&Emulator::ChangeGraphicsConfig, gEmulator)),
                                                  new ItemBase(LANG_RESET_CONFIGS,
                                                               "",
                                                               std::bind(&Ui::_ResetGraphics, this))});

    UpdateControllerOptions();

    std::vector<ItemBase *> hotkeys;
    hotkeys.reserve(HOT_KEY_COUNT + 1);
    for (size_t i = 0; i < HOT_KEY_COUNT; i++)
    {
        hotkeys.emplace_back(new ItemHotkey((HotKeyConfig)i, &gConfig->hotkeys[i]));
    }
    hotkeys.emplace_back(new ItemBase(LANG_RESET_CONFIGS, "", std::bind(&Ui::_ResetHotkey, this)));
    hotkeys[SWITCH_KEYBOARD]->SetVisable(ENABLE_KEYBOARD);
    hotkeys[KEYBOARD_UP]->SetVisable(ENABLE_KEYBOARD);
    hotkeys[KEYBOARD_DOWN]->SetVisable(ENABLE_KEYBOARD);
    _tabs[TAB_INDEX_HOTKEY] = new TabSeletable(LANG_HOTKEY, hotkeys);

    UpdateCoreOptions();

    _tabs[TAB_INDEX_OPTIONS] = new TabSeletable(LANG_OPTIONS, {new ItemConfig(LANG_LANGUAGE,
                                                                              "",
                                                                              (uint32_t *)&gConfig->language,
                                                                              {
                                                                                  LanguageString(gLanguageNames[LANGUAGE_ENGLISH]),
                                                                                  LanguageString(gLanguageNames[LANGUAGE_CHINESE]),
                                                                                  LanguageString(gLanguageNames[LANGUAGE_JAPANESE]),
                                                                                  LanguageString(gLanguageNames[LANGUAGE_ITALIAN]),
                                                                                  LanguageString(gLanguageNames[LANGUAGE_FRENCH]),
                                                                                  LanguageString(gLanguageNames[LANGUAGE_SPANISH]),
                                                                                  LanguageString(gLanguageNames[LANGUAGE_RUSSIAN]),
                                                                              },
                                                                              std::bind(&Ui::_ChangeLanguage, gUi)),
                                                               new ItemTheme(_themes),
                                                               new ItemConfig(LANG_INDEPENDENT_CONFIG,
                                                                              "",
                                                                              &gConfig->independent_config,
                                                                              {LANG_NO, LANG_YES}),
                                                               new ItemConfig(LANG_AUTO_ROTATING,
                                                                              "",
                                                                              &gConfig->auto_rotating,
                                                                              {LANG_NO, LANG_YES},
                                                                              std::bind(&Ui::_ChangeAutoRotating, gUi)),
                                                               new ItemConfig(LANG_REWIND,
                                                                              "",
                                                                              &gConfig->rewind,
                                                                              {LANG_NO, LANG_YES},
                                                                              std::bind(&Emulator::ChangeRewindConfig, gEmulator)),
                                                               new ItemIntConfig(LANG_REWIND_BUF_SIZE,
                                                                                 "",
                                                                                 &gConfig->rewind_buf_size,
                                                                                 MIN_REWIND_BUF_SIZE,
                                                                                 MAX_REWIND_BUF_SIZE,
                                                                                 REWIND_BUF_SIZE_STEP,
                                                                                 std::bind(&Emulator::ChangeRewindConfig, gEmulator)),
                                                               new ItemConfig(LANG_SPEED_STEP,
                                                                              "",
                                                                              &gConfig->speed_step,
                                                                              Emu4VitaPlus::SPEED_STEP_OPTIONS),
                                                               new ItemConfig(LANG_MUTE,
                                                                              "",
                                                                              &gConfig->mute,
                                                                              {LANG_NO, LANG_YES},
                                                                              std::bind(&Emulator::ChangeAudioConfig, gEmulator)),
                                                               new ItemConfig(LANG_AUDIO_LATENCY,
                                                                              "",
                                                                              &gConfig->audio_latency,
                                                                              {"16", "32", "64", "128", "256", "512"}),
                                                               new ItemConfig(LANG_AUTO_SAVE,
                                                                              "",
                                                                              &gConfig->auto_save,
                                                                              {LANG_NO, LANG_YES}),
                                                               new ItemConfig(LANG_AUTO_LOAD,
                                                                              "",
                                                                              &gConfig->auto_load,
                                                                              {LANG_NO, LANG_YES}),
                                                               new ItemConfig(LANG_SWAP_ENTER,
                                                                              "",
                                                                              &gConfig->swap_enter,
                                                                              {LANG_NO, LANG_YES}),
                                                               new ItemConfig(LANG_CPU_FREQ,
                                                                              "",
                                                                              &gConfig->cpu_freq,
                                                                              {LANG_AUTO, "333 MHz", "444 MHz", "500 MHz"},
                                                                              std::bind(&Emulator::SetCpuFreq, gEmulator, -1)),
                                                               new ItemConfig(LANG_SHOW_FPS,
                                                                              "",
                                                                              &gConfig->fps,
                                                                              {LANG_DISABLE,
                                                                               LANG_TOP_LEFT,
                                                                               LANG_TOP_RIGHT,
                                                                               LANG_BOTTOM_LEFT,
                                                                               LANG_BOTTOM_RIGHT}),
                                                               new ItemConfig(LANG_RETROACHIEVEMENTS,
                                                                              LANG_RETROACHIEVEMENTS_DESC,
                                                                              &gConfig->ra_login,
                                                                              {LANG_NO, LANG_YES},
                                                                              std::bind(&Ui::_ChangeRetroArchievements, gUi)),
                                                               new ItemConfig(LANG_HARDCORE,
                                                                              LANG_HARDCORE_DESC,
                                                                              (uint32_t *)&gConfig->ra_hardcore,
                                                                              {LANG_NO, LANG_YES},
                                                                              std::bind(&RetroAchievements::SetHardcoreEnabled, gRetroAchievements, std::ref(gConfig->ra_hardcore))),
                                                               new ItemConfig(LANG_RETROARCHIEVEMENTS_LOCAL,
                                                                              "",
                                                                              (uint32_t *)&gConfig->ra_position,
                                                                              {LANG_TOP_LEFT,
                                                                               LANG_TOP_RIGHT,
                                                                               LANG_BOTTOM_LEFT,
                                                                               LANG_BOTTOM_RIGHT})});

    ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_RETROACHIEVEMENTS)->SetVisable(gRetroAchievements->IsRunning());
    ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_HARDCORE)->SetVisable(gRetroAchievements->IsOnline());
    ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_RETROARCHIEVEMENTS_LOCAL)->SetVisable(gRetroAchievements->IsOnline());

    _tabs[TAB_INDEX_ABOUT] = new TabAbout();

    SetInputHooks();
}