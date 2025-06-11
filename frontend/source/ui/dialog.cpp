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
            if (i == _index)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
            }

            ImGui::Button(option.Get());
            ImGui::SameLine();

            if (i == _index)
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

InputTextDialog::InputTextDialog(const char *title, const char *initial_text)
{
    Utils::Utf8ToUtf16(title, _title, SCE_IME_DIALOG_MAX_TITLE_LENGTH - 1);
    Utils::Utf8ToUtf16(initial_text, _text, SCE_IME_DIALOG_MAX_TITLE_LENGTH - 1);
    *_utf8 = 0;
}

InputTextDialog::~InputTextDialog()
{
    LogFunctionName;
    sceImeDialogTerm();
}

bool InputTextDialog::Init()
{
    LogFunctionName;

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

    if (result != SCE_OK)
    {
        LogWarn("init InputTextDialog failed: %08x", result);
    }
    return result == SCE_OK;
}

bool InputTextDialog::GetStatus()
{
    SceCommonDialogStatus status = sceImeDialogGetStatus();
    if (status == SCE_COMMON_DIALOG_STATUS_FINISHED)
    {
        SceImeDialogResult result{0};
        sceImeDialogGetResult(&result);
        if (result.button == SCE_IME_DIALOG_BUTTON_ENTER)
        {
            Utils::Utf16ToUtf8(_input, _utf8, SCE_IME_DIALOG_MAX_TITLE_LENGTH - 1);
        }
        return true;
    }
    return false;
}