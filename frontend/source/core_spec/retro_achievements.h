#pragma once
#include <rcheevos.h>
#include <rc_client.h>
#include <stdint.h>

class RetroAchievements
{
public:
    RetroAchievements();
    virtual ~RetroAchievements();

    void Login(const char *username, const char *password);
    void LoginWithToekn(const char *username, const char *token);
    void Logout();
    bool IsOnline() { return _online; };
    void LoadGame(const char *path, const uint8_t *rom, size_t rom_size);

private:
    static void _LoginCallback(int result, const char *error_message, rc_client_t *client, void *userdata);
    static void _LoadGameCallback(int result, const char *error_message, rc_client_t *client, void *userdata);

    rc_client_t *_client;
    bool _online;
};