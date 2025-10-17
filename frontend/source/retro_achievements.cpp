#include <stdint.h>
#include "retro_achievements.h"
#include "global.h"
#include "log.h"

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
}

void RetroAchievements::_EventHandler(const rc_client_event_t *event, rc_client_t *client)
{
    LogFunctionName;
    LogDebug("  type: %d", event->type);
    switch (event->type)
    {
    case RC_CLIENT_EVENT_ACHIEVEMENT_TRIGGERED:
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

        gHint->SetHint(TEXT(LANG_LOGIN_SUCCESSFULLY));
        if (gConfig->ra_token.empty())
        {
            const rc_client_user_t *user = rc_client_get_user_info(client);
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
