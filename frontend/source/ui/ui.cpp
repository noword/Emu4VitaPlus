#include <imgui_vita2d/imgui_vita.h>
#include "my_imgui.h"
#include "ui.h"
#include "app.h"
#include "video.h"
#include "log.h"
#include "state_manager.h"
#include "tab_selectable.h"
#include "item_config.h"
#include "item_control.h"
#include "item_hotkey.h"
#include "item_core.h"
#include "item_state.h"
#include "item_device.h"
#include "item_achievement.h"
#include "tab_browser.h"
#include "tab_favorite.h"
#include "tab_about.h"
#include "tab_disk.h"
#include "utils.h"
#include "overlay.h"
#include "shader.h"
#include "defines.h"
#include "global.h"
#include "misc.h"

static void ResumeGame()
{
    LogFunctionName;
    gStatus.Set(APP_STATUS_RUN_GAME);
}

static void ResetGame()
{
    LogFunctionName;
    gEmulator->Reset();
}

static void ExitGame()
{
    LogFunctionName;
    gEmulator->UnloadGame();
}

static void ReturnToArch()
{
    LogFunctionName;
    gEmulator->UnloadGame();
    gStatus.Set(APP_STATUS_RETURN_ARCH);
}

static void ExitApp()
{
    LogFunctionName;
    gEmulator->UnloadGame();
    gStatus.Set(APP_STATUS_EXIT);
}

static void ResetGraphics()
{
    LogFunctionName;
    gConfig->DefaultGraphics();
    gConfig->Save();
}

static void ResetControl()
{
    LogFunctionName;
    gConfig->DefaultControlMap();
    gConfig->Save();
}

static void ResetHotkey()
{
    LogFunctionName;
    gConfig->DefaultHotKey();
    gConfig->Save();
}

static void ResetCoreOptions()
{
    LogFunctionName;
    gConfig->core_options.Default();
    gConfig->Save();
}

void Ui::_InitImgui()
{
    LogFunctionName;

    ImGui::CreateContext();
    My_ImGui_ImplVita2D_Init(LANGUAGE::LANGUAGE_ENGLISH, CACHE_DIR);
    ImGui_ImplVita2D_TouchUsage(false);
    ImGui_ImplVita2D_UseIndirectFrontTouch(false);
    ImGui_ImplVita2D_UseRearTouch(false);
    ImGui_ImplVita2D_GamepadUsage(false);

    ImGuiStyle *style = &ImGui::GetStyle();
    style->Colors[ImGuiCol_TitleBg] = style->Colors[ImGuiCol_TitleBgActive];
    ImVec4 c = style->Colors[ImGuiCol_Tab];
    style->Colors[ImGuiCol_Tab] = {c.x * 0.7, c.y * 0.7, c.z * 0.7, c.w};
    c = style->Colors[ImGuiCol_TabActive];
    style->Colors[ImGuiCol_TabActive] = {c.x * 1.2, c.y * 1.2, c.z * 1.2, c.w};
}

void Ui::_DeinitImgui()
{
    LogFunctionName;
    My_ImGui_ImplVita2D_Shutdown();
    ImGui::DestroyContext();
}

Ui::Ui() : _tab_index(TAB_INDEX_BROWSER),
           _tabs{nullptr},
           _ps_locked(false)
{
    LogFunctionName;
    _boot_ui = new Boot();
    _title = std::string("Emu4Vita++ v") + APP_VER_STR + " (" + gEmulator->GetCoreName() + " " + gEmulator->GetCoreVersion() + ")";
    _InitImgui();
    _dialog = new Dialog("", {LANG_OK, LANG_CANCEL},
                         std::bind(&Ui::_OnDialog, this, std::placeholders::_1, std::placeholders::_2));
}

Ui::~Ui()
{
    LogFunctionName;
    delete _boot_ui;
    delete _dialog;

    _DeinitImgui();
    _ClearTabs();
}

void Ui::_ClearTabs()
{
    for (size_t i = 0; i < TAB_INDEX_COUNT; i++)
    {
        if (_tabs[i] != nullptr)
        {
            delete _tabs[i];
        }
        _tabs[i] = nullptr;
    }
}

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
        new ItemBase(LANG_RESUME_GAME, "", ResumeGame, NULL, false),
        new ItemBase(LANG_RESET_GAME, "", ResetGame, NULL, false),
        new ItemBase(LANG_EXIT_GAME, "", ExitGame, NULL, false),
        new ItemBase(LANG_BACK_TO_ARCH, "", ReturnToArch),
        new ItemBase(LANG_CLEAN_CACHE, "", std::bind(&Ui::_OnCleanCache, this, &_input)),
        new ItemBase(LANG_EXIT, "", ExitApp)};

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
                                                               ResetGraphics)});

    UpdateControllerOptions();

    std::vector<ItemBase *> hotkeys;
    hotkeys.reserve(HOT_KEY_COUNT + 1);
    for (size_t i = 0; i < HOT_KEY_COUNT; i++)
    {
        hotkeys.emplace_back(new ItemHotkey((HotKeyConfig)i, &gConfig->hotkeys[i]));
    }
    hotkeys.emplace_back(new ItemBase(LANG_RESET_CONFIGS, "", ResetHotkey));
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

    ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_RETROACHIEVEMENTS)->SetVisable(gRetroAchievements->Enabled);
    ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_HARDCORE)->SetVisable(gRetroAchievements->IsOnline());
    ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_RETROARCHIEVEMENTS_LOCAL)->SetVisable(gRetroAchievements->IsOnline());

    _tabs[TAB_INDEX_ABOUT] = new TabAbout();

    SetInputHooks();
}

void Ui::SetInputHooks()
{
    _input.SetKeyUpCallback(SCE_CTRL_L1, std::bind(&Ui::_OnKeyL1, this, &_input));
    _input.SetKeyUpCallback(SCE_CTRL_R1, std::bind(&Ui::_OnKeyR1, this, &_input));

    while (_tabs[_tab_index] == nullptr || !_tabs[_tab_index]->Visable())
    {
        _OnKeyL1(&_input);
    }
    _tabs[_tab_index]->SetInputHooks(&_input);
}

void Ui::_OnKeyL1(Input *input)
{
    // LogFunctionName;
    _tabs[_tab_index]->UnsetInputHooks(&_input);

    do
    {
        LOOP_MINUS_ONE(_tab_index, TAB_INDEX_COUNT);
    } while (_tabs[_tab_index] == nullptr || !_tabs[_tab_index]->Visable());

    _tabs[_tab_index]->SetInputHooks(&_input);
}

void Ui::_OnKeyR1(Input *input)
{
    // LogFunctionName;
    _tabs[_tab_index]->UnsetInputHooks(&_input);

    do
    {
        LOOP_PLUS_ONE(_tab_index, TAB_INDEX_COUNT);
    } while (_tabs[_tab_index] == nullptr || !_tabs[_tab_index]->Visable());

    _tabs[_tab_index]->SetInputHooks(&_input);
}

void Ui::_OnPsButton(Input *input)
{
    LogFunctionName;
    ResumeGame();
}

void Ui::Run()
{
    _input.Poll(true);
    gInputTextDialog->Run();
}

void Ui::OnStatusChanged(APP_STATUS status)
{
    LogFunctionName;
    LogDebug("  status changed: to %d", status);

    if (status & (APP_STATUS_SHOW_UI_IN_GAME | APP_STATUS_SHOW_UI))
    {
        LogDebug("  _tab_index: %d", _tab_index);
        LogDebug("  IsMultiDisc: %d", gEmulator->IsMultiDisc());
        gVideo->Lock();

        if (gEmulator->IsMultiDisc())
        {
            if (_tabs[TAB_INDEX_DISK] == nullptr)
            {
                UpdateDiskOptions();
            }
        }
        else
        {
            if (_tabs[TAB_INDEX_DISK] != nullptr)
            {
                delete _tabs[TAB_INDEX_DISK];
                _tabs[TAB_INDEX_DISK] = nullptr;
            }
        }

        _tabs[TAB_INDEX_STATE]->SetVisable(status == APP_STATUS_SHOW_UI_IN_GAME && (!gRetroAchievements->GetHardcoreEnabled()));
        _tabs[TAB_INDEX_ACHIEVEMENTS]->SetVisable(status == APP_STATUS_SHOW_UI_IN_GAME && gRetroAchievements->GetAchievementsCount());
        _tabs[TAB_INDEX_CHEAT]->SetVisable(status == APP_STATUS_SHOW_UI_IN_GAME && (!gRetroAchievements->GetHardcoreEnabled()) && (gEmulator->GetCheats()->size() > 0));
        _tabs[TAB_INDEX_BROWSER]->SetVisable(status == APP_STATUS_SHOW_UI);
        _tabs[TAB_INDEX_FAVORITE]->SetVisable(status == APP_STATUS_SHOW_UI);

        TabSeletable *system_tab = (TabSeletable *)(_tabs[TAB_INDEX_SYSTEM]);
        system_tab->SetItemVisable(0, status == APP_STATUS_SHOW_UI && gConfig->support_no_game);           // LANG_START_CORE_NO_GAME
        system_tab->SetItemVisable(1, status == APP_STATUS_SHOW_UI_IN_GAME);                               // LANG_RESUME_GAME
        system_tab->SetItemVisable(2, status == APP_STATUS_SHOW_UI_IN_GAME && retro_serialize_size() > 0); // LANG_RESET_GAME
        system_tab->SetItemVisable(3, status == APP_STATUS_SHOW_UI_IN_GAME);                               // LANG_EXIT_GAME

        ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_RETROACHIEVEMENTS)->SetVisable(gRetroAchievements->Enabled);

        if (status == APP_STATUS_SHOW_UI_IN_GAME)
        {
            system_tab->SetIndex(0);
            _input.SetKeyUpCallback(gConfig->hotkeys[MENU_TOGGLE], std::bind(&Ui::_OnPsButton, this, &_input));
        }
        else
        {
            _tab_index = TAB_INDEX_BROWSER;
            _input.UnsetKeyUpCallback(gConfig->hotkeys[MENU_TOGGLE]);
        }

        gVideo->Unlock();

        SetInputHooks();
    }

    if (status & (APP_STATUS_RUN_GAME | APP_STATUS_REWIND_GAME | APP_STATUS_SHOW_UI_IN_GAME))
    {
        _LockPsButton();
    }
    else
    {
        _UnlockPsButton();
    }

    LogDebug("OnStatusChanged end");
}

void Ui::_LockPsButton()
{
    if (!_ps_locked)
    {
        LogDebug("  Lock PS button");
        sceShellUtilLock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN |
                                                SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU |
                                                SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION |
                                                SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2));
        _ps_locked = true;
    }
}
void Ui::_UnlockPsButton()
{
    if (_ps_locked)
    {
        LogDebug("  Unlock PS button");
        sceShellUtilUnlock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN |
                                                  SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU |
                                                  SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION |
                                                  SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2));
        _ps_locked = false;
    }
}

void Ui::_ShowNormal()
{
    _tabs[TAB_INDEX_FAVORITE]->SetVisable(gFavorites->size() > 0);
    if (!_tabs[TAB_INDEX_FAVORITE]->Visable() && _tab_index == TAB_INDEX_FAVORITE)
    {
        _OnKeyL1(&_input);
    }

    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTabListScrollingButtons))
    {
        for (size_t i = 0; i < TAB_INDEX_COUNT; i++)
        {
            if (_tabs[i] && _tabs[i]->Visable())
            {
                _tabs[i]->Show(_tab_index == i);
            }
        }
        ImGui::EndTabBar();
    }

    if (_dialog->IsActived())
    {
        _dialog->Show();
    }
}

void Ui::Show()
{
    LogFunctionNameLimited;

#ifndef DRAW_IMGUI_TOGETHER
    ImGui_ImplVita2D_NewFrame();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
#endif
    APP_STATUS status = gStatus.Get();
    ImGui::SetNextWindowPos({MAIN_WINDOW_PADDING, MAIN_WINDOW_PADDING});
    ImGui::SetNextWindowSize({VITA_WIDTH - MAIN_WINDOW_PADDING * 2, VITA_HEIGHT - MAIN_WINDOW_PADDING * 2});

    if (ImGui::Begin(_title.c_str(), NULL,
                     ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        My_ImGui_ShowTimePower(gNetwork->Connected(), gRetroAchievements && gRetroAchievements->IsOnline(), gRetroAchievements->GetHardcoreEnabled());
        (status == APP_STATUS_BOOT) ? _boot_ui->Show() : _ShowNormal();
    }

    ImGui::End();

#ifndef DRAW_IMGUI_TOGETHER
    ImGui::Render();
    My_ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
#endif

    return;
}

void Ui::AppendLog(const char *log)
{
    _boot_ui->AppendLog(log);
}

void Ui::ClearLogs()
{
    LogFunctionName;
    _boot_ui->ClearLogs();
}

void Ui::NotifyBootResult(bool result)
{
    LogFunctionName;
    if (result)
    {
        _tab_index = TAB_INDEX_STATE;
    }
    else
    {
        gHint->SetHint(TEXT(LANG_LOAD_ROM_FAILED));
        if (_boot_ui->GetLogSize() > 0)
        {
            _boot_ui->SetInputHooks(&_input);
        }
        else
        {
            gStatus.Set(APP_STATUS_SHOW_UI);
        }
    }
}

void Ui::UpdateCoreOptions()
{
    LogFunctionName;

    std::vector<ItemBase *> options;
    options.reserve(gConfig->core_options.size() + 1);
    for (auto &iter : gConfig->core_options)
    {
        if (iter.second.values.size() > 0)
        {
            options.emplace_back(new ItemCore(&iter.second));
        }
    }
    options.emplace_back(new ItemBase(LANG_RESET_CONFIGS,
                                      "",
                                      ResetCoreOptions));

    gVideo->Lock();

    if (_tabs[TAB_INDEX_CORE] != nullptr)
    {
        delete _tabs[TAB_INDEX_CORE];
    }

    _tabs[TAB_INDEX_CORE] = new TabSeletable(LANG_CORE, options);

    gVideo->Unlock();
}

void Ui::UpdateCheatOptions()
{
    LogFunctionName;
    std::vector<ItemBase *> options;
    Cheats *cheats = gEmulator->GetCheats();
    options.reserve(cheats->size());
    for (Cheat &cheat : *cheats)
    {
        options.emplace_back(new ItemConfig(cheat.desc,
                                            "",
                                            &cheat.enable,
                                            {LANG_NO, LANG_YES},
                                            std::bind(&Emulator::ChangeCheatConfig, gEmulator)));
    }

    gVideo->Lock();
    if (_tabs[TAB_INDEX_CHEAT] != nullptr)
    {
        delete _tabs[TAB_INDEX_CHEAT];
    }

    _tabs[TAB_INDEX_CHEAT] = new TabSeletable(LANG_CHEAT, options, 2, 0.8);
    gVideo->Unlock();
}

void Ui::UpdateControllerOptions()
{
    LogFunctionName;

    std::vector<ItemBase *> controls;
    controls.reserve(gConfig->control_maps.size() + gConfig->device_options.size() + 3);

    if (gConfig->device_options.size() > 0 && gConfig->device_options[0].size() > 0)
    {
        for (size_t i = 0; i < gConfig->device_options.size(); i++)
        {
            std::string port_name = TEXT(LANG_DEVICE_PORT);
            port_name += std::to_string(i);
            controls.emplace_back(new ItemDevice(port_name, &gConfig->device_options[i]));
        }
    }

    controls.emplace_back(new ItemConfig(LANG_MOUSE,
                                         "",
                                         &gConfig->mouse,
                                         {LANG_DISABLED, LANG_MOUSE_FRONT_TOUCH_PANEL, LANG_MOUSE_REAR_TOUCH_PANEL},
                                         std::bind(&Emulator::SetupKeys, gEmulator)));

    controls.emplace_back(new ItemConfig(LANG_LIGHTGUN,
                                         "",
                                         &gConfig->lightgun,
                                         {LANG_DISABLED, LANG_ENABLED},
                                         std::bind(&Emulator::SetupKeys, gEmulator)));

    controls.emplace_back(new ItemConfig(LANG_SIM_BUTTON,
                                         "",
                                         &gConfig->sim_button,
                                         {LANG_DISABLED, LANG_ENABLED},
                                         std::bind(&Emulator::SetupKeys, gEmulator)));

    for (ControlMapConfig &cmap : gConfig->control_maps)
    {
        controls.emplace_back(new ItemControl(&cmap));
    }

    controls.emplace_back(new ItemBase(LANG_RESET_CONFIGS, "", ResetControl));

    gVideo->Lock();
    if (_tabs[TAB_INDEX_CONTROL] != nullptr)
    {
        delete _tabs[TAB_INDEX_CONTROL];
    }
    _tabs[TAB_INDEX_CONTROL] = new TabSeletable(LANG_CONTROL, controls);
    gVideo->Unlock();
}

void Ui::UpdateDiskOptions()
{
    LogFunctionName;
    DiskControl *disk_control = gEmulator->GetDiskControl();

    gVideo->Lock();
    if (_tabs[TAB_INDEX_DISK] != nullptr)
    {
        delete _tabs[TAB_INDEX_DISK];
        _tabs[TAB_INDEX_DISK] = nullptr;
    }

    if (disk_control && disk_control->GetNumImages() > 1)
    {
        _tabs[TAB_INDEX_DISK] = new TabDisk(disk_control);
    }

    gVideo->Unlock();
}

void Ui::_ChangeLanguage()
{
    LogFunctionName;

    gVideo->Lock();

    LanguageString::Init();

    My_Imgui_Destroy_Font();
    My_Imgui_Create_Font(gConfig->language, CACHE_DIR);
    for (auto tab : _tabs)
    {
        if (tab)
            tab->ChangeLanguage(gConfig->language);
    }
    gConfig->input_descriptors.Update();
    gRomNameMap->Load();

    gVideo->Unlock();

    gConfig->Save();
}

void Ui::_ChangeAutoRotating()
{
    LogFunctionName;

    if (gStatus.Get() == APP_STATUS_SHOW_UI_IN_GAME)
    {
        gVideo->Lock();
        gEmulator->ChangeGraphicsConfig();
        gEmulator->SetupKeys();
        gVideo->Unlock();
    }

    gConfig->Save();
}

void Ui::_OnCleanCache(Input *input)
{
    LogFunctionName;
    _current_dialog = LANG_CLEAN_CACHE;
    _dialog->SetText(TEXT(LANG_CLEAN_CACHE_CONFIRM));
    _dialog->OnActive(input);
}

void Ui::_OnDialog(Input *input, int index)
{
    LogFunctionName;
    LogDebug("_current_dialog: %d index: %d", _current_dialog, index);

    switch (_current_dialog)
    {
    case LANG_CLEAN_CACHE:
        if (index == 0)
        {
            File::RemoveAllFiles(RETRO_ACHIEVEMENTS_CACHE_DIR);
            File::RemoveAllFiles(ARCADE_CACHE_DIR);
            File::RemoveAllFiles(ARCHIVE_CACHE_DIR);
            File::RemoveAllFiles(PLAYLISTS_CACHE_DIR);
            File::RemoveAllFiles(CACHE_DIR);
            if (*THUMBNAILS_PATH)
                File::RemoveAllFiles(THUMBNAILS_PATH);
            gEmulator->ClearCache();
        }
        break;

    default:
        break;
    }
}

void Ui::_OnStartCore(Input *input)
{
    LogFunctionName;
    if (gConfig->support_no_game && gEmulator->LoadRom(NULL, NULL, 0))
    {
        _tabs[TAB_INDEX_SYSTEM]->UnsetInputHooks(input);
    }
}

void Ui::_ChangeRetroArchievements()
{
    LogFunctionName;
    LogDebug("ra_login: %d", gConfig->ra_login);
    if (!gConfig->ra_login)
    {
        gRetroAchievements->Logout();
        OnRetrAchievementsLogInOut(false);
        return;
    }

    if (!gNetwork->Connected())
    {
        _current_dialog = LANG_NO_NETWORK;
        _dialog->SetText(TEXT(LANG_NO_NETWORK));
        _dialog->OnActive(&_input);
        return;
    }

    gInputTextDialog->Open(std::bind(&Ui::_TextInputCallback, this, std::placeholders::_1),
                           TEXT(LANG_USERNAME),
                           gConfig->ra_user.c_str());
    _current_dialog = LANG_USERNAME;
    _input.PushCallbacks();
}

void Ui::_TextInputCallback(const char *text)
{
    LogFunctionName;

    bool pop = true;
    if (*text)
    {
        switch (_current_dialog)
        {
        case LANG_USERNAME:
        {
            if (gConfig->ra_token.empty() || gConfig->ra_user != text)
            {
                gConfig->ra_user = text;
                _current_dialog = LANG_PASSWORD;
                gInputTextDialog->Open(std::bind(&Ui::_TextInputCallback, this, std::placeholders::_1),
                                       TEXT(LANG_PASSWORD));
                pop = false;
            }
            else
            {
                gRetroAchievements->LoginWithToekn(gConfig->ra_user.c_str(), gConfig->ra_token.c_str());
                if (!gRetroAchievements->IsRunning())
                    gRetroAchievements->Start();
            }
        }
        break;

        case LANG_PASSWORD:
        {
            gConfig->ra_password = text;
            gConfig->ra_token.clear();
            gRetroAchievements->Login(gConfig->ra_user.c_str(), gConfig->ra_password.c_str());
            if (!gRetroAchievements->IsRunning())
                gRetroAchievements->Start();
        }
        break;

        default:
            break;
        }
    }

    if (pop)
        _input.PopCallbacks();
}

void Ui::UpdateAchievements()
{
    LogFunctionName;
    size_t count = gRetroAchievements->GetAchievementsCount();

    std::vector<ItemBase *> achievements;

    gVideo->Lock();
    if (count > 0)
    {
        achievements.reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            achievements.emplace_back(new ItemAchievement(gRetroAchievements->GetAchievement(i)));
        }
    }

    if (_tabs[TAB_INDEX_CORE] != nullptr)
    {
        delete _tabs[TAB_INDEX_ACHIEVEMENTS];
    }
    _tabs[TAB_INDEX_ACHIEVEMENTS] = new TabSeletable(LANG_ACHIEVEMENTS, achievements, 1);

    gVideo->Unlock();
}

void Ui::OnRetrAchievementsLogInOut(bool login)
{
    LogFunctionName;

    if (_tabs[TAB_INDEX_OPTIONS])
    {
        ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_HARDCORE)->SetVisable(login);
        ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_RETROARCHIEVEMENTS_LOCAL)->SetVisable(login);
    }
}
