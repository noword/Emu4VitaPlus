#include <stdint.h>
#include "retro_achievements.h"
#include "log.h"

extern "C"
{
    uint32_t ReadMemory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client)
    {
        return 0;
    }

    void HttpCallback(int status_code, const char *content, size_t content_size, void *userdata, const char *error_message)
    {
    }

    void ServerCall(const rc_api_request_t *request, rc_client_server_callback_t callback, void *callback_data, rc_client_t *client)
    {
    }
}

RetroAchievements::RetroAchievements()
{
    LogFunctionName;
    _client = rc_client_create(ReadMemory, ServerCall);
}

RetroAchievements::~RetroAchievements()
{
    LogFunctionName;
    if (_client)
    {
        rc_client_destroy(_client);
    }
}