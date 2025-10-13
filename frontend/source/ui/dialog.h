#pragma once
#include <vector>
#include <imgui_vita2d/imgui_vita.h>
#include "language_string.h"
#include "input.h"

using namespace Emu4VitaPlus;

typedef std::function<void(Input *, int)> DialogCallbackFunc;

class Dialog
{
public:
    Dialog(LanguageString text,
           std::vector<LanguageString> options,
           DialogCallbackFunc callback);
    virtual ~Dialog();

    void Show();
    void SetInputHooks(Input *input);
    void UnsetInputHooks(Input *input);
    void OnActive(Input *input);
    void SetText(LanguageString text);
    bool IsActived() { return _actived; };

private:
    void _OnKeyLeft(Input *input);
    void _OnKeyRight(Input *input);
    void _OnClick(Input *input);
    void _OnCancel(Input *input);

    LanguageString _text;
    std::vector<LanguageString> _options;
    DialogCallbackFunc _callback;
    size_t _index;
    bool _actived;
};

class InputTextDialog
{
public:
    InputTextDialog(const char *title, const char *initial_text = "");
    virtual ~InputTextDialog();

    bool Init();
    void Deinit();
    void SetText(const char *title, const char *initial_text = "");
    bool GetStatus();
    const char *GetInput() { return _utf8; };
    bool Inited() { return _inited; };

private:
    bool _inited;

    uint16_t _title[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
    uint16_t _text[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
    uint16_t _input[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
    char _utf8[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
};