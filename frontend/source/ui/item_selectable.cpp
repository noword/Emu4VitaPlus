#include "my_imgui.h"
#include "item_selectable.h"
#include "defines.h"
#include "utils.h"
#include "icons.h"
#include "log.h"
#include "video.h"

ItemSelectable::ItemSelectable(const LanguageString text,
                               LanguageString info,
                               CallbackFunc active_callback,
                               CallbackFunc option_callback)
    : ItemBase(text, info, active_callback, option_callback),
      _actived(false),
      _index(0)
{
}

ItemSelectable::~ItemSelectable()
{
}

void ItemSelectable::Show(bool selected)
{
    const char *text = GetText();
    ImGui::Selectable(text, selected);
    ImGui::NextColumn();

    if (_IsOnOff())
    {
        _ShowOnOff(strcasecmp(_GetPreviewText(), TEXT(LANG_YES)) == 0 ||
                   strcasecmp(_GetPreviewText(), TEXT(LANG_ENABLED)) == 0);
    }
    else
    {
        _ShowCombo(text);
    }
}

void ItemSelectable::_ShowCombo(const char *text)
{
    bool is_popup = ImGui::IsPopupOpen(text);

    if (_actived && !is_popup)
    {
        ImGui::OpenPopup(text);
    }

    if (My_ImGui_BeginCombo(text, _GetPreviewText(), ImGuiComboFlags_NoArrowButton))
    {
        if (!_actived && is_popup)
        {
            ImGui::CloseCurrentPopup();
        }
        for (size_t i = 0; i < _GetTotalCount(); i++)
        {
            ImGui::Selectable(_GetOptionString(i), i == _GetIndex());
            if (i == _GetIndex() && ImGui::GetScrollMaxY() > 0.f)
            {
                ImGui::SetScrollHereY((float)i / (float)_GetTotalCount());
            }
        }
        ImGui::EndCombo();
    }
}

void ItemSelectable::_ShowOnOff(bool on)
{
    on ? ImGui::TextUnformatted(ICON_ON) : ImGui::TextDisabled(ICON_OFF);
}

void ItemSelectable::SetInputHooks(Input *input)
{
    input->SetKeyDownCallback(SCE_CTRL_UP, std::bind(&ItemSelectable::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&ItemSelectable::_OnKeyDown, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_UP, std::bind(&ItemSelectable::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_DOWN, std::bind(&ItemSelectable::_OnKeyDown, this, input), true);
    input->SetKeyUpCallback(EnterButton, std::bind(&ItemSelectable::_OnClick, this, input));
    input->SetKeyUpCallback(CancelButton, std::bind(&ItemSelectable::_OnCancel, this, input));
}

void ItemSelectable::UnsetInputHooks(Input *input)
{
    input->UnsetKeyDownCallback(SCE_CTRL_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_DOWN);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_DOWN);
    input->UnsetKeyUpCallback(SCE_CTRL_CIRCLE);
    input->UnsetKeyUpCallback(SCE_CTRL_CROSS);
}

void ItemSelectable::_OnKeyUp(Input *input)
{
    size_t index = _GetIndex();
    LOOP_MINUS_ONE(index, _GetTotalCount());
    _SetIndex(index);
}

void ItemSelectable::_OnKeyDown(Input *input)
{
    size_t index = _GetIndex();
    LOOP_PLUS_ONE(index, _GetTotalCount());
    _SetIndex(index);
}

void ItemSelectable::_OnClick(Input *input)
{
    gVideo->Lock();
    _actived = false;
    if (!_IsOnOff())
        input->PopCallbacks();
    ItemBase::OnActive(input);
    gVideo->Unlock();
}

void ItemSelectable::_OnCancel(Input *input)
{
    gVideo->Lock();
    _actived = false;
    input->PopCallbacks();
    _SetIndex(_old_index);
    gVideo->Unlock();
}

void ItemSelectable::OnActive(Input *input)
{
    if (_IsOnOff())
    {
        _OnKeyDown(input);
        _OnClick(input);
    }
    else
    {
        gVideo->Lock();
        _actived = true;
        _old_index = _GetIndex();
        input->PushCallbacks();
        SetInputHooks(input);
        gVideo->Unlock();
    }
}

#define TEST(A, B) ((strcasecmp(_GetOptionString(0), TEXT(A)) == 0 && strcasecmp(_GetOptionString(1), TEXT(B)) == 0) || \
                    (strcasecmp(_GetOptionString(0), TEXT(B)) == 0 && strcasecmp(_GetOptionString(1), TEXT(A)) == 0))

bool ItemSelectable::_IsOnOff()
{
    if (_GetTotalCount() == 2)
    {
        if (TEST(LANG_YES, LANG_NO) || TEST(LANG_ENABLED, LANG_DISABLED))
        {
            return true;
        }
    }

    return false;
}