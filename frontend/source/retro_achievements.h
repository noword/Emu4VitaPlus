#pragma once
#include <rcheevos.h>
#include <rc_client.h>
#include <stdint.h>
#include <string>
#include <map>
#include <vita2d.h>
#include "thread_base.h"
#include "log.h"

// 5 seconds
#define RETRO_ACHIEVEMENTS_LOGIN_IDLE_TIME 5000000
// 0.95 second
#define RETRO_ACHIEVEMENTS_IDLE_TIME 950000
// 5 seconds
#define RETRO_ACHIEVEMENTS_DISPLAY_TIME 5000000

struct Notification
{
    Notification() : texture(nullptr), _stop_time(0) {};

    virtual ~Notification()
    {
        if (texture)
            vita2d_free_texture(texture);
    }

    bool TimeUp() const
    {
        return _stop_time != 0 && sceKernelGetSystemTimeWide() >= _stop_time;
    }

    void SetShowTime(uint64_t time = RETRO_ACHIEVEMENTS_DISPLAY_TIME)
    {
        _stop_time = sceKernelGetSystemTimeWide() + time;
    }

    std::string title;
    std::string text;
    vita2d_texture *texture;

private:
    uint64_t _stop_time;
};

class RetroAchievements : public ThreadBase
{
public:
    RetroAchievements();
    virtual ~RetroAchievements();

    void Show();
    bool IsOnline() { return _online; };
    void Login(const char *username, const char *password);
    void LoginWithToekn(const char *username, const char *token);
    void Logout();
    void LoadGame(const char *path, const void *rom, size_t rom_size);
    void UnloadGame();
    void Reset();
    void AddNotification(uint32_t id, Notification *n);
    void RemoveNotification(uint32_t id);
    void UpdateeNotification(uint32_t id, const std::string &title, const std::string &text = "", vita2d_texture *texture = nullptr);

private:
    static int
    _RaThread(SceSize args, void *argp);

    static uint32_t _ReadMemory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client);
    static void _ServerCall(const rc_api_request_t *request, rc_client_server_callback_t callback, void *callback_data, rc_client_t *client);
    static void _EventHandler(const rc_client_event_t *event, rc_client_t *client);
    static void _LogMessage(const char *message, const rc_client_t *client);
    static void _LoginCallback(int result, const char *error_message, rc_client_t *client, void *userdata);
    static void _LoadGameCallback(int result, const char *error_message, rc_client_t *client, void *userdata);

    void _ClearNotifictions();

    rc_client_t *_client;
    bool _online;
    std::map<uint32_t, Notification *> _notifications;
};