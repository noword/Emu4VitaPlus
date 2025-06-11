#include <string>
#include "tab_selectable.h"
#include "defines.h"
#include "utils.h"
#include "global.h"

TabSeletable::TabSeletable(TEXT_ENUM title_id, std::vector<ItemBase *> items, int columns, float column_ratio)
    : TabBase(title_id),
      _items(std::move(items)),
      _index(0),
      _columns(columns),
      _column_ratio(column_ratio),
      _in_refreshing(false)
{
    LogFunctionName;
}

TabSeletable::TabSeletable(TEXT_ENUM title_id, int columns)
    : TabBase(title_id),
      _index(0),
      _columns(columns),
      _column_ratio(-1.f),
      _in_refreshing(false)
{
    LogFunctionName;
}

TabSeletable::~TabSeletable()
{
    LogFunctionName;
    for (auto &item : _items)
    {
        delete item;
    }
}

void TabSeletable::SetInputHooks(Input *input)
{
    input->SetKeyDownCallback(SCE_CTRL_UP, std::bind(&TabSeletable::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&TabSeletable::_OnKeyDown, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_UP, std::bind(&TabSeletable::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_DOWN, std::bind(&TabSeletable::_OnKeyDown, this, input), true);
    input->SetKeyUpCallback(SCE_CTRL_LEFT, std::bind(&TabSeletable::_OnKeyLeft, this, input));
    input->SetKeyUpCallback(SCE_CTRL_RIGHT, std::bind(&TabSeletable::_OnKeyRight, this, input));
    input->SetKeyUpCallback(EnterButton, std::bind(&TabSeletable::_OnActive, this, input));
    input->SetKeyUpCallback(SCE_CTRL_TRIANGLE, std::bind(&TabSeletable::_OnOption, this, input));
}

void TabSeletable::UnsetInputHooks(Input *input)
{
    input->UnsetKeyDownCallback(SCE_CTRL_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_DOWN);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_DOWN);
    input->UnsetKeyUpCallback(SCE_CTRL_LEFT);
    input->UnsetKeyUpCallback(SCE_CTRL_RIGHT);
    input->UnsetKeyUpCallback(EnterButton);
    input->UnsetKeyUpCallback(SCE_CTRL_TRIANGLE);
}

void TabSeletable::Show(bool selected)
{
    std::string title = std::string(TAB_ICONS[_title_id]) + TEXT(_title_id);
    if (ImGui::BeginTabItem(title.c_str(), NULL, selected ? ImGuiTabItemFlags_SetSelected : 0))
    {
        ImVec2 size = {0.f, 0.f};
        float avail_width = ImGui::GetContentRegionAvailWidth();
        if (_status_text.size() > 0)
        {
            ImVec2 s = ImGui::CalcTextSize(_status_text.c_str(), NULL, false, avail_width);
            size.y = -s.y;
        }

        if (ImGui::BeginChild(TEXT(_title_id), size))
        {
            ImGui::Columns(_columns, NULL, false);

            if (_columns == 2 && _column_ratio > 0)
            {
                ImGui::SetColumnOffset(1, avail_width * _column_ratio);
            }

            if (_in_refreshing)
            {
                _spin_text.Show();
            }
            else
            {
                size_t total = _GetItemCount();
                ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(36, 36, 36, 255));
                for (size_t i = 0; i < total; i++)
                {
                    if (_ItemVisable(i))
                    {
                        _ShowItem(i, i == _index);
                        if (i == _index && ImGui::GetScrollMaxY() > 0.f)
                        {
                            ImGui::SetScrollHereY((float)_index / (float)total);
                        }
                        ImGui::NextColumn();
                    }
                    else if (i == _index)
                    {
                        LOOP_PLUS_ONE(_index, total);
                    }
                }
                ImGui::PopStyleColor();
            }
            ImGui::Columns(1);
        }

        ImGui::EndChild();
        if (_status_text.size() > 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::TextWrapped("%s", _status_text.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::EndTabItem();
    }
}

void TabSeletable::_OnKeyUp(Input *input)
{
    if (_GetItemCount() == 0)
    {
        return;
    }

    _moving_status.Reset();

    do
    {
        LOOP_MINUS_ONE(_index, _GetItemCount());
    } while (!_ItemVisable(_index));

    _Update();
}

void TabSeletable::_OnKeyDown(Input *input)
{
    if (_GetItemCount() == 0)
    {
        return;
    }

    _moving_status.Reset();

    do
    {
        LOOP_PLUS_ONE(_index, _GetItemCount());
    } while (!_ItemVisable(_index));

    _Update();
}

void TabSeletable::_OnKeyLeft(Input *input)
{
    if (_GetItemCount() == 0)
    {
        return;
    }

    _moving_status.Reset();

    if (_index < 10)
    {
        _index = 0;
    }
    else
    {
        size_t index = _index - 10;
        while ((!_ItemVisable(index)) && index > 0)
        {
            index--;
        }
        _index = index;
    }

    _Update();
}

void TabSeletable::_OnKeyRight(Input *input)
{
    if (_GetItemCount() == 0)
    {
        return;
    }

    _moving_status.Reset();

    size_t index = _index + 10;
    if (index >= _GetItemCount())
    {
        index = _GetItemCount() - 1;
    }
    else
    {
        while ((!_ItemVisable(index)) && index < _GetItemCount())
        {
            index++;
        }
    }

    _index = index;

    _Update();
}

size_t TabSeletable::_GetItemCount()
{
    return _items.size();
}

void TabSeletable::_ShowItem(size_t index, bool selected)
{
    if (index < _items.size())
    {
        _items[index]->Show(selected);
    }
}

void TabSeletable::_OnActive(Input *input)
{
    LogFunctionName;
    if (_index < _items.size())
    {
        _items[_index]->OnActive(input);
    }
}

void TabSeletable::_OnOption(Input *input)
{
    LogFunctionName;
    if (_index < _items.size())
    {
        _items[_index]->OnOption(input);
    }
}

bool TabSeletable::_ItemVisable(size_t index)
{
    if (index < _items.size())
    {
        return _items[index]->Visable();
    }
    else
    {
        return false;
    }
}

void TabSeletable::SetStatusText(std::string text)
{
    _status_text = text;
}

void TabSeletable::SetItemVisable(size_t index, bool visable)
{
    if (index < _items.size())
    {
        _items[index]->SetVisable(visable);
    }
}

// bool TabSeletable::ItemVisable(size_t index)
// {
//     if (index < _items.size())
//     {
//         return _items[index]->Visable();
//     }
//     else
//     {
//         return false;
//     }
// }

void TabSeletable::SetColumns(int columns)
{
    _columns = columns;
}

void TabSeletable::SetIndex(size_t index)
{
    if (index >= 0 && index < _items.size())
    {
        _index = index;
    }
}

void TabSeletable::_Update()
{
    gVideo->Lock();
    if (_ItemVisable(_index))
    {
        _status_text = _items[_index]->GetInfo();
    }
    gVideo->Unlock();
}