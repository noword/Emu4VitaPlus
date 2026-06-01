#include <imgui_vita2d/imgui_vita.h>
#include "my_imgui.h"
#include "ui.h"
#include "log.h"
#include "state_manager.h"
#include "tab_selectable.h"
#include "tab_control.h"
#include "item_config.h"
#include "item_control.h"
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
#include "global.h"
#include "misc.h"

void Ui::_ResumeGame()
{
    LogFunctionName;
    gStatus.Set(APP_STATUS_RUN_GAME);
}

void Ui::_ResetGame()
{
    LogFunctionName;
    gEmulator->Reset();
}

void Ui::_ExitGame()
{
    LogFunctionName;
    gEmulator->UnloadGame();
}

void Ui::_ReturnToArch()
{
    LogFunctionName;
    gEmulator->UnloadGame();
    gStatus.Set(APP_STATUS_RETURN_ARCH);
}

void Ui::_ExitApp()
{
    LogFunctionName;
    gEmulator->UnloadGame();
    gStatus.Set(APP_STATUS_EXIT);
}

void Ui::_ResetGraphics()
{
    LogFunctionName;
    gConfig->DefaultGraphics();
    gConfig->Save();
}

void Ui::_ResetControl()
{
    LogFunctionName;
    gConfig->DefaultControlMap();
    gConfig->Save();
}

void Ui::_ResetHotkey()
{
    LogFunctionName;
    gConfig->DefaultHotKey();
    gConfig->Save();
}

void Ui::_ResetCoreOptions()
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

    _themes.Load(DEFAULT_THEMES_JSON);
    _themes.Apply(gConfig->theme.c_str());
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
    _ResumeGame();
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

        ((TabSeletable *)_tabs[TAB_INDEX_OPTIONS])->GetItemByLanguageString(LANG_RETROACHIEVEMENTS)->SetVisable(gRetroAchievements->IsRunning());

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
        (gStatus.Get() == APP_STATUS_BOOT) ? _boot_ui->Show() : _ShowNormal();
    }

    ImGui::End();

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
                                      std::bind(&Ui::_ResetCoreOptions, this)));

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

    controls.emplace_back(new ItemConfig(LANG_REAR_SIM_BUTTON,
                                         "",
                                         &gConfig->sim_button_rear,
                                         {LANG_DISABLED, LANG_ENABLED},
                                         std::bind(&Emulator::SetupKeys, gEmulator)));

    controls.emplace_back(new ItemConfig(LANG_FRONT_SIM_BUTTON,
                                         "",
                                         &gConfig->sim_button_front,
                                         {LANG_DISABLED, LANG_ENABLED},
                                         std::bind(&Emulator::SetupKeys, gEmulator)));

    for (ControlMapConfig &cmap : gConfig->control_maps)
    {
        controls.emplace_back(new ItemControl(&cmap));
    }

    controls.emplace_back(new ItemBase(LANG_RESET_CONFIGS, "", std::bind(&Ui::_ResetControl, this)));

    gVideo->Lock();
    if (_tabs[TAB_INDEX_CONTROL] != nullptr)
    {
        delete _tabs[TAB_INDEX_CONTROL];
    }
    _tabs[TAB_INDEX_CONTROL] = new TabControl(controls);
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
    if (gConfig->support_no_game && gEmulator->LoadRom(EMPTY_ROM_NAME, NULL, 0))
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
