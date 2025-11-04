#include <stdint.h>
#include "retro_achievements.h"
#include "global.h"
#include "file.h"
#include "log.h"

int RetroAchievements::_RaThread(SceSize args, void *argp)
{
    LogFunctionName;
    CLASS_POINTER(RetroAchievements, ra, argp);

    uint32_t idle_time;
    APP_STATUS status = gStatus.Get();
    while (ra->IsRunning() && (status & (APP_STATUS_EXIT | APP_STATUS_RETURN_ARCH | APP_STATUS_REBOOT_WITH_LOADING)) == 0)
    {
        if (gNetwork->Connected() && ra->_online)
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
        }
        else
        {
            idle_time = RETRO_ACHIEVEMENTS_LOGIN_IDLE_TIME;
        }

        ra->Wait(&idle_time);
        status = gStatus.Get();
    }

    LogDebug("_RaThread exit");
    sceKernelExitThread(0);
    return 0;
}

uint32_t RetroAchievements::_ReadMemory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client)
{
    if (gRetroAchievements->_retro_memory == nullptr)
    {
        const rc_client_game_t *game = rc_client_get_game_info(gRetroAchievements->_client);
        gRetroAchievements->_retro_memory = new RetroMemory(&gRetroAchievements->_mmap, game ? game->console_id : 0);
    }

    return gRetroAchievements->_retro_memory->Read(address, buffer, num_bytes);
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
    ra->_ClearAchievemnts();

    Notification *notification = new Notification;

    const rc_client_game_t *game = rc_client_get_game_info(ra->_client);
    LogInfo("[RA]         id: %d", game->id);
    LogInfo("[RA]      title: %s", game->title);
    LogInfo("[RA]       hash: %s", game->hash);
    ra->_game_id = game->id;

    notification->title = game->title;

    if (rc_client_game_get_image_url(game, url, sizeof(url)) == RC_OK)
    {
        notification->texture = ra->_texture_cache.Get(url, game->id, GAME_IMAGE_ID);
    }

    rc_client_user_game_summary_t summary;
    rc_client_get_user_game_summary(ra->_client, &summary);
    notification->text = std::to_string(summary.num_unlocked_achievements) + " / " + std::to_string(summary.num_core_achievements);

    notification->SetShowTime();
    gNotifications->Add(game->id, notification);

    if (rc_client_is_processing_required(ra->_client))
    {
        ra->_UpdateAchievemnts();
        ra->SetHardcoreEnabled(gConfig->ra_hardcore);
    }
    else
    {
        ra->SetHardcoreEnabled(false);
    }
}

void RetroAchievements::_EventHandler(const rc_client_event_t *event, rc_client_t *client)
{
    LogFunctionName;
    LogDebug("  type: %d", event->type);

    if (event->type > RC_CLIENT_EVENT_TYPE_NONE &&
        event->type < RC_CLIENT_EVENT_SUBSET_COMPLETED &&
        RetroAchievements::_event_functions[event->type])
    {
        (gRetroAchievements->*RetroAchievements::_event_functions[event->type])(event);
    }
}

RetroAchievements::RetroAchievements()
    : ThreadBase(_RaThread),
      _online(false),
      _mmap{0},
      _retro_memory(nullptr),
      Enabled(false)
{
    LogFunctionName;

    File::MakeDirs(RETRO_ACHIEVEMENTS_CACHE_DIR);

    _client = rc_client_create(_ReadMemory, _ServerCall);
    rc_client_enable_logging(_client, RC_CLIENT_LOG_LEVEL_VERBOSE, _LogMessage);
    rc_client_set_event_handler(_client, _EventHandler);
}

RetroAchievements::~RetroAchievements()
{
    LogFunctionName;

    _ClearAchievemnts();
    _ClearRetroMmap();

    if (_retro_memory)
        delete _retro_memory;

    if (_client)
    {
        if (_online)
        {
            Logout();
        }
        rc_client_destroy(_client);
    }
}

void RetroAchievements::CopyRetroMmap(const retro_memory_map *mmap)
{
    LogFunctionName;

    _ClearRetroMmap();
    if (_retro_memory)
    {
        delete _retro_memory;
        _retro_memory = nullptr;
    }

    retro_memory_descriptor *descriptors = new retro_memory_descriptor[mmap->num_descriptors];
    _mmap.num_descriptors = mmap->num_descriptors;
    _mmap.descriptors = descriptors;

    LogDebug("index, flags, offset, start, select, disconnect, len, addrspace");
    const retro_memory_descriptor *md = mmap->descriptors;
    for (auto i = 0; i < _mmap.num_descriptors; i++)
    {
        LogDebug("%d %llx %08x %08x %08x %08x %08x %s", i, md->flags, md->offset, md->start, md->select, md->disconnect, md->len, md->addrspace ? md->addrspace : "");
        memcpy(descriptors++, md++, sizeof(retro_memory_descriptor));
    }
}

void RetroAchievements::_ClearRetroMmap()
{
    LogFunctionName;
    if (_mmap.descriptors)
    {
        delete[] _mmap.descriptors;
        _mmap.descriptors = nullptr;
        _mmap.num_descriptors = 0;
    }
}

void RetroAchievements::_LoginCallback(int result, const char *error_message, rc_client_t *client, void *userdata)
{
    LogFunctionName;
    RetroAchievements *ra = (RetroAchievements *)userdata;
    gConfig->ra_login = ra->_online = (result == RC_OK);
    gUi->OnRetrAchievementsLogInOut(ra->_online);
    if (ra->_online)
    {
        const rc_client_user_t *user = rc_client_get_user_info(client);

        Notification *notification = new Notification;
        notification->texture = ra->_texture_cache.Get(user->avatar_url);
        notification->title = TEXT(LANG_LOGIN_SUCCESSFUL);
        notification->text = std::string(user->display_name) + " / " + std::to_string(user->score);
        notification->SetShowTime();
        gNotifications->Add(0, notification);

        if (gConfig->ra_token.empty())
        {
            gConfig->ra_token = user->token;
        }

        ra->SetHardcoreEnabled(gConfig->ra_hardcore);
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
        gHardcore = _online = false;
        Stop();
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
    _ClearAchievemnts();
}

void RetroAchievements::Reset()
{
    LogFunctionName;
    rc_client_reset(_client);
}

void RetroAchievements::SetHardcoreEnabled(const bool &enabled)
{
    LogFunctionName;
    LogDebug("  enabled: %d", enabled);
    gHardcore = enabled;
    rc_client_set_hardcore_enabled(_client, enabled);
}

void RetroAchievements::_UpdateAchievemnts()
{
    LogFunctionName;

    char url[128];
    rc_client_achievement_list_t *list = rc_client_create_achievement_list(_client,
                                                                           RC_CLIENT_ACHIEVEMENT_CATEGORY_CORE_AND_UNOFFICIAL,
                                                                           RC_CLIENT_ACHIEVEMENT_LIST_GROUPING_PROGRESS);
    for (int i = 0; i < list->num_buckets; i++)
    {
        LogDebug("%d bucket_type: %d", i, list->buckets[i].bucket_type);
        if (list->buckets[i].bucket_type == RC_CLIENT_ACHIEVEMENT_BUCKET_UNSUPPORTED)
            continue;

        for (int j = 0; j < list->buckets[i].num_achievements; j++)
        {
            const rc_client_achievement_t *achievement = list->buckets[i].achievements[j];
            LogDebug("  [%d, %d] %s (%d, %d, %d)", i, j, achievement->title, achievement->id, achievement->state, achievement->unlocked);
            LogDebug("  %s", achievement->description);

            if (strcmp(achievement->title, "Warning: Unknown Emulator") == 0 ||
                strcmp(achievement->title, "Unsupported Game Version") == 0)
                continue;

            Achievement *a = new Achievement(_game_id, achievement->id, achievement->unlocked);
            a->title = achievement->title;
            a->description = achievement->description;
            if (achievement->unlocked)
            {
                char buf[32];
                strftime(buf, 32, "%Y-%m-%d %H:%M:%S", localtime(&achievement->unlock_time));
                a->unlock_time = buf;
                LogDebug("  unlock time: %s", buf);
            }

            for (int state : {RC_CLIENT_ACHIEVEMENT_STATE_ACTIVE, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED})
            {
                std::string img_path = a->GetImagePath(state == RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED);
                if ((!File::Exist(img_path.c_str())) && rc_client_achievement_get_image_url(achievement, state, url, sizeof(url)) == RC_OK)
                {
                    gNetwork->AddTask(url, img_path.c_str());
                }
            }

            _achievements[achievement->id] = a;
        }
    }
    rc_client_destroy_achievement_list(list);

    gUi->UpdateAchievements();
}

void RetroAchievements::_ClearAchievemnts()
{
    LogFunctionName;

    gVideo->Lock();
    for (auto a : _achievements)
    {
        delete a.second;
    }
    _achievements.clear();
    gNotifications->Clear();
    gVideo->Unlock();
}

Achievement *RetroAchievements::GetAchievement(size_t index)
{
    LogFunctionName;

    if (index >= 0 && index < _achievements.size())
    {
        auto it = _achievements.begin();
        std::advance(it, index);
        return it->second;
    }
    else
    {
        return nullptr;
    }
}