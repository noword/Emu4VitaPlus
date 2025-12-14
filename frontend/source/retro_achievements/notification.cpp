#include "notification.h"
#include "global.h"
#include "log.h"

#define NOTIFY_WINDOW_WIDTH 180.f
#define NOTIFY_WINDOW_HEIGHT 70.f
#define NOTIFY_IMAGE_HEIGHT (NOTIFY_WINDOW_HEIGHT * 0.76f)

void Notifications::Clear()
{
    LogFunctionName;

    _locker.Lock();

    if (!_notifications.empty())
    {
        for (auto &n : _notifications)
        {
            delete n.second;
        }
        _notifications.clear();
    }

    _locker.Unlock();
}

static void _SetNextWindowPosition(ImVec2 &pos, const ImVec2 &size, ImVec2 &pre_size)
{
    if (pos.x < 0)
    {
        // first setting
        switch (gConfig->ra_position)
        {
        case RA_POSITION_TOP_LEFT:
            pos.x = 0.f;
            pos.y = 0.f;
            break;

        case RA_POSITION_TOP_RIGHT:
            pos.x = VITA_WIDTH - size.x;
            pos.y = 0.f;
            break;

        case RA_POSITION_BOTTOM_LEFT:
            pos.x = 0.f;
            pos.y = VITA_HEIGHT - size.y;
            break;

        case RA_POSITION_BOTTOM_RIGHT:
            pos.x = VITA_WIDTH - size.x;
            pos.y = VITA_HEIGHT - size.y;
            break;

        default:
            break;
        }
    }
    else
    {
        switch (gConfig->ra_position)
        {
        case RA_POSITION_TOP_LEFT:
            pos.y += pre_size.y;
            break;

        case RA_POSITION_TOP_RIGHT:
            pos.x = VITA_WIDTH - size.x;
            pos.y += pre_size.y;
            break;

        case RA_POSITION_BOTTOM_LEFT:
            pos.y -= size.y;
            break;

        case RA_POSITION_BOTTOM_RIGHT:
            pos.x = VITA_WIDTH - size.x;
            pos.y -= size.y;
            break;

        default:
            break;
        }
    }
}

void Notifications::Show()
{
    if (_notifications.empty())
        return;

    ImVec2 pos{-1.f, -1.f};
    ImVec2 size;
    ImVec2 pre_size{0.f, 0.f};

    ImGui_ImplVita2D_NewFrame();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);

    _locker.Lock();
    for (const auto &iter : _notifications)
    {
        const auto n = iter.second;
        if (n->TimeUp())
            continue;

        float image_width = n->texture ? NOTIFY_IMAGE_HEIGHT * 1.3 : 0.f;
        float title_width = n->title.empty() ? 0 : (ImGui::CalcTextSize(n->title.c_str()).x + 25);
        float text_width = n->title.empty() ? 0 : (ImGui::CalcTextSize(n->text.c_str()).x + 25);

        size = {std::max(image_width + title_width, image_width + text_width), NOTIFY_WINDOW_HEIGHT};
        if ((!n->texture) && (n->title.empty() || n->text.empty()))
        {
            size.y *= 0.6;
        }

        _SetNextWindowPosition(pos, size, pre_size);
        pre_size = size;

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowBgAlpha(0.8f);
        if (ImGui::Begin((n->title + std::to_string(pos.x) + std::to_string(pos.y)).c_str(),
                         NULL,
                         ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_NoFocusOnAppearing))
        {
            ImGui::BeginGroup();
            if (n->texture)
            {
                float ratio = NOTIFY_IMAGE_HEIGHT / vita2d_texture_get_height(n->texture);
                ImGui::Image(n->texture, {vita2d_texture_get_height(n->texture) * ratio, NOTIFY_IMAGE_HEIGHT});
            }
            ImGui::SameLine();
            ImGui::BeginGroup();

            ImGui::Text(n->title.c_str());

            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
            ImGui::Text(n->text.c_str());
            ImGui::PopStyleColor();

            ImGui::EndGroup();
            ImGui::EndGroup();
        }
        ImGui::End();
    }
    _locker.Unlock();

    ImGui::Render();
    My_ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
}

void Notifications::Run()
{
    if (likely(_notifications.empty()))
        return;

    for (const auto &iter : _notifications)
    {
        if (!iter.second->TimeUp())
        {
            return;
        }
    }

    Clear();
}

void Notifications::Add(uint32_t id, Notification *n)
{
    LogFunctionName;
    LogDebug("  %08x: '%s' / '%s' / %08x", id, n->title.c_str(), n->text.c_str(), n->texture);

    _locker.Lock();
    _notifications[id] = n;
    _locker.Unlock();
}

void Notifications::Remove(uint32_t id)
{
    LogFunctionName;
    LogDebug("  id: %d", id);

    _locker.Lock();
    auto iter = _notifications.find(id);
    if (iter != _notifications.end())
    {
        iter->second->SetShowTime(0);
    }
    _locker.Unlock();
}

void Notifications::Update(uint32_t id, const std::string &title, const std::string &text, vita2d_texture *texture)
{
    LogFunctionName;
    _locker.Lock();
    auto iter = _notifications.find(id);
    if (iter != _notifications.end())
    {
        auto n = iter->second;
        if (!title.empty())
            n->title = title;

        if (!text.empty())
            n->text = text;

        if (texture)
            n->texture = texture;
    }
    _locker.Unlock();
}