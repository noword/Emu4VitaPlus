#pragma once
#include <psp2/kernel/threadmgr.h>
#include <vector>
#include <string>
#include <language_string.h>
#include "emulator.h"
#include "input.h"
#include "tab_base.h"
#include "dialog.h"
#include "boot.h"
#include "global.h"

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
    void NotificationBootFailed();
    void ChangeLanguage();
    void UpdateCoreOptions();
    void UpdateCheatOptions();
    void UpdateControllerOptions();
    void UpdateDiskOptions();
    void OnStatusChanged(APP_STATUS status);
    void SetHint(const char *s, int frame_count = 120);

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

    void _ShowNormal();
    void _ShowHint();

    void _ClearTabs();

    std::string _title;

    Input _input;
    size_t _tab_index;

    TabBase *_tabs[TAB_INDEX_COUNT];
    Dialog *_dialog;
    int _current_dialog;
    Boot *_boot_ui;

    LanguageString _hint;
    int _hint_count;
};

extern Ui *gUi;