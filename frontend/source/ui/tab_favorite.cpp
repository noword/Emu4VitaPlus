#include <math.h>
#include <string>
#include "tab_favorite.h"
#include "file.h"
#include "config.h"
#include "emulator.h"
#include "video.h"
#include "misc.h"
#include "ra_lpl.h"

TabFavorite::TabFavorite()
    : TabSeletable(LANG_FAVORITE),
      _texture(nullptr),
      _texture_max_width(BROWSER_TEXTURE_MAX_WIDTH),
      _texture_max_height(BROWSER_TEXTURE_MAX_HEIGHT)
{
    std::string name = File::GetName(gConfig->last_rom.c_str());
    size_t count = 0;
    for (const auto &fav : *gFavorites)
    {
        if (name == fav.first)
        {
            _index = count;
            break;
        }
        count++;
    }

    _UpdateStatus();
    _UpdateTexture();

    _dialog = new Dialog{LANG_REMOVE_FAVORITE_CONFIRM,
                         {LANG_OK, LANG_CANCEL},
                         std::bind(&TabFavorite::_OnDialog, this, std::placeholders::_1, std::placeholders::_2)};
}

TabFavorite::~TabFavorite()
{
    delete _dialog;
}

void TabFavorite::Show(bool selected)
{
    std::string title = std::string(TAB_ICONS[_title_id]) + TEXT(_title_id);
    if (ImGui::BeginTabItem(title.c_str(), NULL, selected ? ImGuiTabItemFlags_SetSelected : 0))
    {
        if (_index >= gFavorites->size())
        {
            _index = 0;
        }

        ImVec2 size = {0.f, 0.f};
        ImVec2 s = ImGui::CalcTextSize(_status_text.c_str());
        size.y = -s.y * (s.x / ImGui::GetContentRegionAvailWidth() + 1);

        if (ImGui::BeginChild(TEXT(_title_id), size))
        {
            ImGui::Columns(2, NULL, false);

            auto iter = gFavorites->begin();
            std::advance(iter, _index);
            ImGui::TextUnformatted(iter->second.path.c_str());

            size_t count = 0;
            const float total = gFavorites->size();
            const std::string *rom_name = nullptr;
            if (ImGui::ListBoxHeader("", ImGui::GetContentRegionAvail()))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                for (const auto &fav : *gFavorites)
                {
                    if (count == _index)
                    {
                        My_ImGui_Selectable(fav.second.item.name.c_str(), true, &_moving_status);
                    }
                    else
                    {
                        ImGui::Selectable(fav.second.item.name.c_str());
                    }

                    if (count == _index)
                    {
                        rom_name = &fav.second.rom_name;

                        if (ImGui::GetScrollMaxY() > 0.f)
                            ImGui::SetScrollHereY((float)_index / total);
                    }
                    count++;
                }
                ImGui::PopStyleColor();
                ImGui::ListBoxFooter();
            }

            ImGui::NextColumn();

            ImVec2 avail_size = ImGui::GetContentRegionAvail();
            _texture_max_width = avail_size.x;
            _texture_max_height = avail_size.y;

            ImVec2 pos = ImGui::GetCursorScreenPos();
            if (_texture != nullptr)
            {
                ImVec2 texture_pos = pos;
                texture_pos.x += ceilf(fmax(0.0f, (avail_size.x - _texture_width) * 0.5f));
                texture_pos.y += ceilf(fmax(0.0f, (avail_size.y - _texture_height) * 0.5f));
                ImGui::SetCursorScreenPos(texture_pos);
                ImGui::Image(_texture, {_texture_width, _texture_height});
            }

            if (rom_name && rom_name->size() > 0)
            {
                _name_moving_status.Update(rom_name->c_str());
                ImVec2 text_size = ImGui::CalcTextSize(rom_name->c_str());
                ImVec2 text_pos = pos;
                text_pos.x += fmax(0, (avail_size.x - text_size.x) / 2) + _name_moving_status.pos;
                text_pos.y += (_texture == nullptr ? (avail_size.y - text_size.y) / 2 : 10);

                if (_texture)
                {
                    My_ImGui_HighlightText(rom_name->c_str(), text_pos, IM_COL32_GREEN, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)));
                }
                else
                {
                    ImGui::SetCursorScreenPos(text_pos);
                    ImGui::TextUnformatted(rom_name->c_str());
                }
            }

            if (_info.size() > 0)
            {
                ImVec2 text_size = ImGui::CalcTextSize(_info.c_str());
                ImVec2 text_pos = pos;

                if (_texture)
                {
                    text_pos.y += avail_size.y - text_size.y - 10;
                    My_ImGui_HighlightText(_info.c_str(), text_pos, IM_COL32_GREEN, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)));
                }
                else
                {
                    text_pos.y += (avail_size.y - text_size.y) / 2;
                    if (rom_name && rom_name->size() > 0)
                    {
                        text_pos.y += text_size.y;
                    }
                    ImGui::SetCursorScreenPos(text_pos);
                    ImGui::TextUnformatted(_info.c_str());
                }
            }

            ImGui::NextColumn();
        }

        ImGui::EndChild();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xcc, 0xcc, 0xcc, 255));
        ImGui::TextWrapped(_status_text.c_str());
        ImGui::PopStyleColor();

        ImGui::EndTabItem();
    }

    if (_dialog->IsActived())
    {
        _dialog->Show();
    }
}

void TabFavorite::SetInputHooks(Input *input)
{
    TabSeletable::SetInputHooks(input);

    input->SetKeyUpCallback(CancelButton, std::bind(&TabFavorite::_OnKeyCross, this, input));
}

void TabFavorite::UnsetInputHooks(Input *input)
{
    TabSeletable::UnsetInputHooks(input);
    input->UnsetKeyUpCallback(CancelButton);
}

void TabFavorite::_OnActive(Input *input)
{
    LogFunctionName;

    auto iter = gFavorites->begin();
    std::advance(iter, _index);
    const Favorite &fav = iter->second;

    if (gEmulator->LoadRom((fav.path + "/" + fav.item.name).c_str(), fav.item.entry_name.c_str(), fav.item.crc32))
    {
        UnsetInputHooks(input);
    }
}

void TabFavorite::_OnKeyCross(Input *input)
{
    LogFunctionName;
    _dialog->OnActive(input);
}

void TabFavorite::_UpdateStatus()
{
    _status_text = EnterButton == SCE_CTRL_CIRCLE ? BUTTON_CIRCLE : BUTTON_CROSS;
    _status_text += TEXT(LANG_START_GAME);
    _status_text += "\t";
    _status_text += EnterButton == SCE_CTRL_CIRCLE ? BUTTON_CROSS : BUTTON_CIRCLE;
    _status_text += TEXT(LANG_REMOVE_FAVORITE);
}

void TabFavorite::_OnDialog(Input *input, int index)
{
    LogFunctionName;
    if (index == 0)
    {
        gVideo->Lock();
        auto iter = gFavorites->begin();
        std::advance(iter, _index);
        gFavorites->erase(iter);
        gFavorites->Save();
        gVideo->Unlock();
    }
}

void TabFavorite::ChangeLanguage(uint32_t language)
{
    LogFunctionName;
    _UpdateStatus();
}

void TabFavorite::_UpdateTexture()
{
    if (_texture != nullptr)
    {
        gVideo->Lock();
        vita2d_wait_rendering_done();
        vita2d_free_texture(_texture);
        _texture = nullptr;
        gVideo->Unlock();
    }

    if (gFavorites->size() == 0)
    {
        return;
    }

    auto iter = gFavorites->begin();
    std::advance(iter, _index);
    const Favorite &fav = iter->second;

    if (gPlaylists->IsValid())
    {
        _texture = gPlaylists->GetPreviewImage((fav.path + "/" + fav.item.name).c_str());
    }

    if (_texture == nullptr)
    {
        _texture = GetRomPreviewImage(fav.path.c_str(), fav.item.name.c_str());
    }

    if (_texture)
    {
        CalcFitSize(vita2d_texture_get_width(_texture),
                    vita2d_texture_get_height(_texture),
                    _texture_max_width,
                    _texture_max_height,
                    &_texture_width,
                    &_texture_height);
    }
}

void TabFavorite::_UpdateInfo()
{
    if (_info.size() > 0)
    {
        gVideo->Lock();
        _info = "";
        gVideo->Unlock();
    }

    auto iter = gFavorites->begin();
    std::advance(iter, _index);
    const Favorite &fav = iter->second;
    std::string full_path = fav.path + "/" + fav.item.name;
    gVideo->Lock();
    _info = GetFileInfoString(full_path.c_str());
    gVideo->Unlock();
}

void TabFavorite::_Update()
{
    _UpdateStatus();
    _UpdateTexture();
    _UpdateInfo();
    _moving_status.Reset();
    _name_moving_status.Reset();
}
