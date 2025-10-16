#pragma once
#include <rcheevos.h>
#include <rc_client.h>
#include <stdint.h>

class RetroAchievements
{
public:
    RetroAchievements();
    virtual ~RetroAchievements();

    bool IsOnline() { return _online; };
    void Login(const char *username, const char *password);
    void LoginWithToekn(const char *username, const char *token);
    void Logout();
    void LoadGame(const char *path, const void *rom, size_t rom_size);

private:
    static uint32_t _ReadMemory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client);
    static void _ServerCall(const rc_api_request_t *request, rc_client_server_callback_t callback, void *callback_data, rc_client_t *client);
    static void _EventHandler(const rc_client_event_t *event, rc_client_t *client);
    static void _LogMessage(const char *message, const rc_client_t *client);
    static void _LoginCallback(int result, const char *error_message, rc_client_t *client, void *userdata);
    static void _LoadGameCallback(int result, const char *error_message, rc_client_t *client, void *userdata);

    rc_client_t *_client;
    bool _online;
};