#include <stdint.h>
#include "retro_achievements.h"
#include "global.h"
#include "log.h"

#define NOTIFY_WINDOW_WIDTH 180.f
#define NOTIFY_WINDOW_HEIGHT 70.f
#define NOTIFY_IMAGE_HEIGHT (NOTIFY_WINDOW_HEIGHT * 0.76f)

int RetroAchievements::_RaThread(SceSize args, void *argp)
{
    LogFunctionName;
    CLASS_POINTER(RetroAchievements, ra, argp);

    uint32_t idle_time;
    APP_STATUS status = gStatus.Get();
    while (ra->IsRunning() && (status & (APP_STATUS_EXIT | APP_STATUS_RETURN_ARCH | APP_STATUS_REBOOT_WITH_LOADING)) == 0)
    {
        if (gNetwork->Connected())
        {
            if (ra->_online)
            {
                switch (status)
                {
                case APP_STATUS_RUN_GAME:
                    rc_client_do_frame(ra->_client);
                    break;

                case APP_STATUS_SHOW_UI_IN_GAME:
                    rc_client_idle(ra->_client);
                    break;

                default:
                    break;
                }
                idle_time = RETRO_ACHIEVEMENTS_IDLE_TIME;
                ra->Wait(&idle_time);
            }
            else if (gConfig->ra_login && !gConfig->ra_token.empty())
            {
                ra->LoginWithToekn(gConfig->ra_user.c_str(), gConfig->ra_token.c_str());
                idle_time = RETRO_ACHIEVEMENTS_LOGIN_IDLE_TIME;
                ra->Wait(&idle_time);
            }
        }
        else
        {
            idle_time = RETRO_ACHIEVEMENTS_LOGIN_IDLE_TIME;
            ra->Wait(&idle_time);
        }

        status = gStatus.Get();
    }

    LogDebug("_RaThread exit");
    sceKernelExitThread(0);
    return 0;
}

uint32_t RetroAchievements::_ReadMemory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client)
{
    // LogFunctionName;
    // LogDebug("  address: %08x num_bytes: %08x", address, num_bytes);
    size_t size = retro_get_memory_size(RETRO_MEMORY_SYSTEM_RAM);
    uint8_t *data = (uint8_t *)retro_get_memory_data(RETRO_MEMORY_SYSTEM_RAM);
    if (address + num_bytes > size || !data)
    {
        return 0;
    }

    data += address;

    memcpy(buffer, data, num_bytes);

    return num_bytes;
}

void RetroAchievements::_ServerCall(const rc_api_request_t *request, rc_client_server_callback_t callback, void *callback_data, rc_client_t *client)
{
    if (request->post_data)
    {
        gNetwork->AddTask(request->url, request->post_data, strlen(request->post_data), (ClientCallBackFunc)callback, callback_data);
    }
    else
    {
        gNetwork->AddTask(request->url, (ClientCallBackFunc)callback, callback_data);
    }
}

void RetroAchievements::_LogMessage(const char *message, const rc_client_t *client)
{
    LogInfo("[rc client] %s", message);
}

void RetroAchievements::_LoadGameCallback(int result, const char *error_message, rc_client_t *client, void *userdata)
{
    LogFunctionName;
    char url[128];
    RetroAchievements *ra = (RetroAchievements *)userdata;

    Notification *notification = new Notification;

    const rc_client_game_t *game = rc_client_get_game_info(ra->_client);
    notification->title = game->title;
    std::string buf;
    if (rc_client_game_get_image_url(game, url, sizeof(url)) == RC_OK && gNetwork->Fetch(url, &buf))
    {
        notification->texture = vita2d_load_PNG_buffer(buf.c_str());
    }

    rc_client_user_game_summary_t summary;
    rc_client_get_user_game_summary(ra->_client, &summary);
    notification->text = std::to_string(summary.num_unlocked_achievements) + " / " + std::to_string(summary.num_core_achievements);

    notification->SetShowTime();
    gRetroAchievements->AddNotification(game->id, notification);
}

void RetroAchievements::_EventHandler(const rc_client_event_t *event, rc_client_t *client)
{
    LogFunctionName;
    LogDebug("  type: %d", event->type);

    switch (event->type)
    {
    case RC_CLIENT_EVENT_ACHIEVEMENT_TRIGGERED:
    {
        auto achievement = event->achievement;
        Notification *notification = new Notification;
        notification->title = TEXT(LANG_ACHIEVEMENT_UNLOCKED);
        notification->text = achievement->title;

        char url[128];
        std::string buf;
        if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, url, sizeof(url)) == RC_OK && gNetwork->Fetch(url, &buf))
        {
            notification->texture = vita2d_load_PNG_buffer(buf.c_str());
        }
        notification->SetShowTime();
        gRetroAchievements->AddNotification(achievement->id, notification);
    }
    break;

    case RC_CLIENT_EVENT_LEADERBOARD_STARTED:
    {
        auto leaderboard = event->leaderboard;
        Notification *notification = new Notification;
        notification->title = TEXT(LANG_LEADERBOARD_STARTED);
        notification->text = leaderboard->title;
        notification->SetShowTime();
        gRetroAchievements->AddNotification(leaderboard->id, notification);
    }
    break;

    case RC_CLIENT_EVENT_LEADERBOARD_FAILED:
    {
        auto leaderboard = event->leaderboard;
        Notification *notification = new Notification;
        notification->title = TEXT(LANG_LEADERBOARD_FAILED);
        notification->text = leaderboard->title;
        notification->SetShowTime();
        gRetroAchievements->AddNotification(leaderboard->id, notification);
    }
    break;

    case RC_CLIENT_EVENT_LEADERBOARD_SUBMITTED:
    {
        auto leaderboard = event->leaderboard;
        Notification *notification = new Notification;
        notification->title = TEXT(LANG_LEADERBOARD_SUBMITTED);
        notification->text = leaderboard->title;
        notification->SetShowTime();
        gRetroAchievements->AddNotification(leaderboard->id, notification);
    }
    break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_SHOW:
    {
        auto achievement = event->achievement;
        Notification *notification = new Notification;
        char url[128];
        std::string buf;
        if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, url, sizeof(url)) == RC_OK && gNetwork->Fetch(url, &buf))
        {
            notification->texture = vita2d_load_PNG_buffer(buf.c_str());
        }
        gRetroAchievements->AddNotification(achievement->id, notification);
    }
    break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_HIDE:
    {
        auto achievement = event->achievement;
        gRetroAchievements->RemoveNotification(achievement->id);
    }
    break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_SHOW:
    {
        auto achievement = event->achievement;
        Notification *notification = new Notification;
        notification->title = achievement->measured_progress;
        notification->text = achievement->title;

        char url[128];
        std::string buf;
        if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, url, sizeof(url)) == RC_OK && gNetwork->Fetch(url, &buf))
        {
            notification->texture = vita2d_load_PNG_buffer(buf.c_str());
        }
        gRetroAchievements->AddNotification(achievement->id, notification);
    }
    break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_HIDE:
    {
        auto achievement = event->achievement;
        gRetroAchievements->RemoveNotification(achievement->id);
    }
    break;

    case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_UPDATE:
    {
        {
            auto achievement = event->achievement;
            gRetroAchievements->UpdateeNotification(achievement->id, achievement->measured_progress);
        }
    }
    break;

    case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_SHOW:
    {
        auto tracker = event->leaderboard_tracker;
        Notification *notification = new Notification;
        notification->title = tracker->display;
        gRetroAchievements->AddNotification(tracker->id, notification);
    }
    break;

    case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_HIDE:
    {
        auto tracker = event->leaderboard_tracker;
        gRetroAchievements->RemoveNotification(tracker->id);
    }
    break;

    case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_UPDATE:
    {
        auto tracker = event->leaderboard_tracker;
        gRetroAchievements->UpdateeNotification(tracker->id, tracker->display);
    }
    break;

    case RC_CLIENT_EVENT_LEADERBOARD_SCOREBOARD:
        break;
    case RC_CLIENT_EVENT_RESET:
        break;
    case RC_CLIENT_EVENT_GAME_COMPLETED:
        break;
    case RC_CLIENT_EVENT_SERVER_ERROR:
        break;

    case RC_CLIENT_EVENT_DISCONNECTED:
        gRetroAchievements->_online = false;
        break;

    case RC_CLIENT_EVENT_RECONNECTED:
        gRetroAchievements->_online = true;
        break;

    case RC_CLIENT_EVENT_SUBSET_COMPLETED:
        break;
    default:
        break;
    }
}

RetroAchievements::RetroAchievements() : ThreadBase(_RaThread), _online(false)
{
    LogFunctionName;
    _client = rc_client_create(_ReadMemory, _ServerCall);
    rc_client_enable_logging(_client, RC_CLIENT_LOG_LEVEL_VERBOSE, _LogMessage);
    rc_client_set_event_handler(_client, _EventHandler);
    rc_client_set_hardcore_enabled(_client, 0);
}

RetroAchievements::~RetroAchievements()
{
    LogFunctionName;

    _ClearNotifictions();

    if (_client)
    {
        if (_online)
        {
            Logout();
        }
        rc_client_destroy(_client);
    }
}

void RetroAchievements::_LoginCallback(int result, const char *error_message, rc_client_t *client, void *userdata)
{
    LogFunctionName;
    RetroAchievements *ra = (RetroAchievements *)userdata;
    gConfig->ra_login = ra->_online = (result == RC_OK);
    if (ra->_online)
    {
        Notification *notification = new Notification;

        const rc_client_user_t *user = rc_client_get_user_info(client);
        std::string buf;
        if (gNetwork->Fetch(user->avatar_url, &buf))
        {
            notification->texture = vita2d_load_PNG_buffer(buf.c_str());
        }
        else
        {
            LogDebug("Fetch avatar failed");
        }

        notification->title = TEXT(LANG_LOGIN_SUCCESSFUL);
        notification->text = std::string(user->display_name) + " / " + std::to_string(user->score);
        notification->SetShowTime();
        ra->AddNotification(0, notification);

        // Notification *n = new Notification;
        // n->title = "test";
        // ra->AddNotification(1, n);

        // n = new Notification;
        // n->texture = vita2d_load_PNG_buffer(buf.c_str());
        // ra->AddNotification(2, n);

        // n = new Notification;
        // n->title = "test";
        // n->text = "fdsafdsafdafdsafd  safdsaf";
        // n->texture = vita2d_load_PNG_buffer(buf.c_str());
        // ra->AddNotification(3, n);

        if (gConfig->ra_token.empty())
        {
            gConfig->ra_token = user->token;
        }
    }
    else
    {
        LogWarn("Login failed: %s", error_message);
        if (gConfig->ra_token.empty())
        {
            gHint->SetHint(TEXT(LANG_LOGIN_FAILED));
        }
        else
        {
            gHint->SetHint(TEXT(LANG_TOKEN_EXPIRED));
            gConfig->ra_token.clear();
        }
    }

    gConfig->Save();
}

void RetroAchievements::Login(const char *username, const char *password)
{
    LogFunctionName;
    rc_client_begin_login_with_password(_client, username, password, _LoginCallback, this);
}

void RetroAchievements::LoginWithToekn(const char *username, const char *token)
{
    LogFunctionName;
    rc_client_begin_login_with_token(_client, username, token, _LoginCallback, this);
}

void RetroAchievements::Logout()
{
    LogFunctionName;
    if (_online)
    {
        rc_client_logout(_client);
        _online = false;
    }
}

void RetroAchievements::LoadGame(const char *path, const void *rom, size_t rom_size)
{
    LogFunctionName;
    rc_client_begin_identify_and_load_game(_client,
                                           RC_CONSOLE_UNKNOWN,
                                           path,
                                           (const uint8_t *)rom,
                                           rom_size,
                                           _LoadGameCallback,
                                           this);
}

void RetroAchievements::UnloadGame()
{
    LogFunctionName;
    rc_client_unload_game(_client);
}

void RetroAchievements::Reset()
{
    LogFunctionName;
    rc_client_reset(_client);
}

static void _SetNextWindowPosition(ImVec2 &pos, const ImVec2 &size)
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
            pos.y += size.y;
            break;

        case RA_POSITION_TOP_RIGHT:
            pos.x = VITA_WIDTH - size.x;
            pos.y += size.y;
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

void RetroAchievements::Show()
{
    if (_notifications.empty())
        return;

    ImGui_ImplVita2D_NewFrame();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    ImVec2 pos{-1.f, -1.f};
    ImVec2 size;

    size_t show_count = 0;
    Lock();
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

        _SetNextWindowPosition(pos, size);
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowBgAlpha(0.8f);
        if (ImGui::Begin((n->title + std::to_string(pos.x) + std::to_string(pos.y)).c_str(),
                         NULL,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDecoration))
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

        show_count++;
    }
    Unlock();

    ImGui::Render();
    My_ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());

    if (show_count == 0)
    {
        _ClearNotifictions();
    }
}

void RetroAchievements::_ClearNotifictions()
{
    Lock();
    for (auto &n : _notifications)
    {
        delete n.second;
    }
    _notifications.clear();
    Unlock();
}

void RetroAchievements::AddNotification(uint32_t id, Notification *n)
{
    LogFunctionName;
    LogDebug("  %08x: %s / %s / %08x", n->title.c_str(), n->text.c_str(), n->texture);

    Lock();
    _notifications[id] = n;
    Unlock();
}

void RetroAchievements::RemoveNotification(uint32_t id)
{
    Lock();
    auto iter = _notifications.find(id);
    if (iter != _notifications.end())
    {
        delete iter->second;
        _notifications.erase(iter);
    }
    Unlock();
}

void RetroAchievements::UpdateeNotification(uint32_t id, const std::string &title, const std::string &text, vita2d_texture *texture)
{
    Lock();
    auto iter = _notifications.find(id);
    if (iter != _notifications.end())
    {
        auto n = iter->second;
        n->title = title;
        n->text = text;
        n->texture = texture;
    }
    Unlock();
}
