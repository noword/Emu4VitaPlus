#include <stdint.h>
#include "retro_achievements.h"
#include "global.h"
#include "log.h"

#if defined(VBA_NEXT_BUILD) ||              \
    defined(GAMBATTE_BUILD) ||              \
    defined(FBNEO_BUILD) ||                 \
    defined(FCEUMM_BUILD) ||                \
    defined(GENESIS_PLUS_GX_BUILD) ||       \
    defined(MEDNAFEN_PCE_FAST_BUILD) ||     \
    defined(MEDNAFEN_SUPERGRAFX_BUILD) ||   \
    defined(MEDNAFEN_NGP_BUILD) ||          \
    defined(MEDNAFEN_WSWAN_BUILD) ||        \
    defined(PROSYSTEM_BUILD) ||             \
    defined(VECX_BUILD) ||                  \
    defined(NEOCD_BUILD) ||                 \
    defined(KM_FBNEO_XTREME_AMPED_BUILD) || \
    defined(BLUEMSX_BUILD) ||               \
    defined(MEDNAFEN_LYNX_BUILD) ||         \
    defined(HANDY_BUILD) ||                 \
    defined(CAP32_BUILD)
#define SUPPORT_RETRO_ACHIEVEMENTS
#endif

uint32_t ReadMemory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client)
{
    return 0;
}

void ServerCall(const rc_api_request_t *request, rc_client_server_callback_t callback, void *callback_data, rc_client_t *client)
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

void LogMessage(const char *message, const rc_client_t *client)
{
    LogInfo("rc client log: %s", message);
}

RetroAchievements::RetroAchievements() : _online(false)
{
    LogFunctionName;
    _client = rc_client_create(ReadMemory, ServerCall);
    rc_client_enable_logging(_client, RC_CLIENT_LOG_LEVEL_VERBOSE, LogMessage);
    rc_client_set_hardcore_enabled(_client, 0);
}

RetroAchievements::~RetroAchievements()
{
    LogFunctionName;
    if (_client)
    {
        rc_client_destroy(_client);
    }
}

void RetroAchievements::_LoginCallback(int result, const char *error_message, rc_client_t *client, void *userdata)
{
    LogFunctionName;
    if (result != RC_OK)
    {
        LogWarn("Login failed: %s", error_message);
        return;
    }

    RetroAchievements *ra = (RetroAchievements *)userdata;
    ra->_online = true;
    const rc_client_user_t *user = rc_client_get_user_info(client);
    // store_retroachievements_credentials(user->username, user->token);

    LogDebug("Logged in as %s (%u points)", user->display_name, user->score);
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

void RetroAchievements::_LoadGameCallback(int result, const char *error_message, rc_client_t *client, void *userdata)
{
    LogFunctionName;
}

void RetroAchievements::LoadGame(const char *path, const uint8_t *rom, size_t rom_size)
{
    LogFunctionName;
    rc_client_begin_identify_and_load_game(_client,
                                           RC_CONSOLE_UNKNOWN,
                                           path,
                                           rom,
                                           rom_size,
                                           _LoadGameCallback,
                                           this);
}
