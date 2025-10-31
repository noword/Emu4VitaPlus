#pragma once
#include <psp2/kernel/threadmgr.h>
#include <language_string.h>
#include "emulator.h"
#include "input.h"
#include "tab_base.h"
#include "dialog.h"
#include "boot.h"
#include "app_status.h"

class Ui
{
    friend class App;

public:
    Ui();
    virtual ~Ui();

    void CreateTables(); // don't call it before gEmulator initialized
    void Show();
    void Run();
    void SetInputHooks();
    void AppendLog(const char *log);
    void ClearLogs();
    void NotifyBootResult(bool result);

    void UpdateCoreOptions();
    void UpdateCheatOptions();
    void UpdateControllerOptions();
    void UpdateDiskOptions();
    void UpdateAchievements();
    void OnStatusChanged(APP_STATUS status);
    void OnRetrAchievementsLogInOut(bool login);

private:
    void
    _InitImgui();
    void _DeinitImgui();

    void _OnKeyL1(Input *input);
    void _OnKeyR1(Input *input);
    void _OnPsButton(Input *input);
    void _OnDialog(Input *input, int index);
    void _OnCleanCache(Input *input);
    void _OnStartCore(Input *input);

    void _ChangeLanguage();
    void _ChangeAutoRotating();
    void _ChangeRetroArchievements();
    void _ChangeHardcore();
    void _TextInputCallback(const char *text);

    void _ShowNormal();
    void _ShowHint();

    void _ClearTabs();
    void _LockPsButton();
    void _UnlockPsButton();

    std::string _title;

    Input _input;
    size_t _tab_index;

    TabBase *_tabs[TAB_INDEX_COUNT];
    Dialog *_dialog;
    int _current_dialog;
    Boot *_boot_ui;
    bool _ps_locked;
};
