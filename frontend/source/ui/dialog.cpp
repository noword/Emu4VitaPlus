#include "dialog.h"
#include "defines.h"
#include "utils.h"
#include "global.h"

Dialog::Dialog(LanguageString text,
               std::vector<LanguageString> options,
               DialogCallbackFunc callback)
    : _text(text),
      _options(std::move(options)),
      _callback(callback),
      _index(0),
      _actived(false)
{
}

Dialog::~Dialog()
{
}

void Dialog::Show()
{
    bool is_popup = ImGui::IsPopupOpen("Dialog");
    if (_actived && !is_popup)
    {
        ImGui::OpenPopup("Dialog");
    }

    if (ImGui::BeginPopupModal("Dialog", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (!_actived && is_popup)
        {
            ImGui::CloseCurrentPopup();
        }

        const char *text = _text.Get();
        if (*text)
        {
            ImGui::TextUnformatted(_text.Get());
            ImGui::Separator();
        }

        size_t i = 0;
        for (const auto &option : _options)
        {
            bool selected = (i == _index);
            if (selected)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            }

            ImGui::Button(option.Get());
            ImGui::SameLine();

            if (selected)
            {
                ImGui::PopStyleColor();
            }

            i++;
        }

        ImGui::EndPopup();
    }
}

void Dialog::SetText(LanguageString text)
{
    _text = text;
}

void Dialog::OnActive(Input *input)
{
    gVideo->Lock();
    _actived = true;
    input->PushCallbacks();
    SetInputHooks(input);
    gVideo->Unlock();
}

void Dialog::SetInputHooks(Input *input)
{
    input->SetKeyDownCallback(SCE_CTRL_LEFT, std::bind(&Dialog::_OnKeyLeft, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_RIGHT, std::bind(&Dialog::_OnKeyRight, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_LEFT, std::bind(&Dialog::_OnKeyLeft, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_RIGHT, std::bind(&Dialog::_OnKeyRight, this, input), true);
    input->SetKeyUpCallback(EnterButton, std::bind(&Dialog::_OnClick, this, input));
    input->SetKeyUpCallback(CancelButton, std::bind(&Dialog::_OnCancel, this, input));
}

void Dialog::UnsetInputHooks(Input *input)
{
    input->UnsetKeyDownCallback(SCE_CTRL_LEFT);
    input->UnsetKeyDownCallback(SCE_CTRL_RIGHT);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_LEFT);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_RIGHT);
    input->UnsetKeyUpCallback(SCE_CTRL_CIRCLE);
    input->UnsetKeyUpCallback(SCE_CTRL_CROSS);
}

void Dialog::_OnKeyLeft(Input *input)
{
    LOOP_MINUS_ONE(_index, _options.size());
}

void Dialog::_OnKeyRight(Input *input)
{
    LOOP_PLUS_ONE(_index, _options.size());
}

void Dialog::_OnClick(Input *input)
{
    gVideo->Lock();
    _actived = false;
    input->PopCallbacks();
    if (_callback)
    {
        _callback(input, _index);
    }
    gVideo->Unlock();
}

void Dialog::_OnCancel(Input *input)
{
    gVideo->Lock();
    _actived = false;
    input->PopCallbacks();
    gVideo->Unlock();
}

InputTextDialog::InputTextDialog() : _inited(false)
{
}

InputTextDialog::~InputTextDialog()
{
    LogFunctionName;
    Close();
}

bool InputTextDialog::Open(InputDialogCallbackFunc callback, const char *title, const char *initial_text)
{
    LogFunctionName;
    LogDebug("  title: %s  initial_text: %s  callback: %08x", title, initial_text, callback);

    Close();

    _callback = callback;
    Utils::Utf8ToUtf16(title, _title, SCE_IME_DIALOG_MAX_TITLE_LENGTH - 1);
    Utils::Utf8ToUtf16(initial_text, _text, SCE_IME_DIALOG_MAX_TITLE_LENGTH - 1);
    *_utf8 = 0;

    SceImeDialogParam param;
    sceImeDialogParamInit(&param);

    param.supportedLanguages = 0x0001FFFF;
    param.languagesForced = SCE_FALSE;
    param.type = SCE_IME_TYPE_DEFAULT;
    param.option = 0;
    param.title = _title;
    param.maxTextLength = SCE_IME_DIALOG_MAX_TITLE_LENGTH;
    param.initialText = _text;
    param.inputTextBuffer = _input;

    int32_t result = sceImeDialogInit(&param);
    _inited = (result == SCE_OK);
    if (!_inited)
    {
        LogWarn("init InputTextDialog failed: %08x", result);
    }
    return _inited;
}

void InputTextDialog::Close()
{
    LogFunctionName;
    if (_inited)
    {
        sceImeDialogTerm();
        _inited = false;
    }
}

void InputTextDialog::Run()
{
    if (!_inited)
        return;

    if (sceImeDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_FINISHED)
    {
        SceImeDialogResult result{0};
        sceImeDialogGetResult(&result);
        if (result.button == SCE_IME_DIALOG_BUTTON_ENTER)
        {
            Utils::Utf16ToUtf8(_input, _utf8, SCE_IME_DIALOG_MAX_TITLE_LENGTH - 1);
        }
        else
        {
            *_utf8 = '\x00';
        }

        if (_callback(_utf8))
        {
            Close();
        }
    }
}