#pragma once
#include <rcheevos.h>
#include <rc_client.h>
#include <libretro.h>
#include <stdint.h>
#include <string>
#include <map>
#include <vita2d.h>
#include "thread_base.h"
#include "achievement.h"
#include "notification.h"
#include "retro_memory.h"
#include "log.h"
#include "texture_cache.h"

// 5 seconds
#define RETRO_ACHIEVEMENTS_LOGIN_IDLE_TIME 5000000
// 0.95 second
#define RETRO_ACHIEVEMENTS_IDLE_TIME 950000

#define GAME_IMAGE_ID 0xffffffff

class RetroAchievements : public ThreadBase
{
public:
    RetroAchievements();
    virtual ~RetroAchievements();

    void CopyRetroMmap(const retro_memory_map *mmap);
    bool IsOnline() { return _online; };
    void Login(const char *username, const char *password);
    void LoginWithToekn(const char *username, const char *token);
    void Logout();
    void LoadGame(const char *path, const void *rom, size_t rom_size);
    void UnloadGame();
    void Reset();
    void SetHardcoreEnabled(const bool &enabled);
    bool GetHardcoreEnabled();
    void ClearTextureCache();

    size_t GetAchievementsCount() { return _achievements.size(); };
    Achievement *GetAchievement(size_t index);

    bool Enabled;

private:
    static int
    _RaThread(SceSize args, void *argp);

    static uint32_t _ReadMemory(uint32_t address, uint8_t *buffer, uint32_t num_bytes, rc_client_t *client);
    static void _ServerCall(const rc_api_request_t *request, rc_client_server_callback_t callback, void *callback_data, rc_client_t *client);
    static void _EventHandler(const rc_client_event_t *event, rc_client_t *client);
    static void _LogMessage(const char *message, const rc_client_t *client);
    static void _LoginCallback(int result, const char *error_message, rc_client_t *client, void *userdata);
    static void _LoadGameCallback(int result, const char *error_message, rc_client_t *client, void *userdata);

    void _ClearRetroMmap();
    void _ClearRetroMemory();
    void _UpdateAchievemnts();
    void _ClearAchievemnts();

    typedef void (RetroAchievements::*EventFunc)(const rc_client_event_t *event);
    static EventFunc _event_functions[];

    void _OnAchievementTriggered(const rc_client_event_t *event);
    void _OnLeaderboardStarted(const rc_client_event_t *event);
    void _OnLeaderboardFailed(const rc_client_event_t *event);
    void _OnLeaderboardSubmitted(const rc_client_event_t *event);
    void _OnAchievementChallengeIndicatorShow(const rc_client_event_t *event);
    void _OnAchievementChallengeIndicatorHide(const rc_client_event_t *event);
    void _OnAchievementProgressIndicatorShow(const rc_client_event_t *event);
    void _OnAchievementProgressIndicatorHide(const rc_client_event_t *event);
    void _OnAchievementProgressIndicatorUpdate(const rc_client_event_t *event);
    void _OnLeaderboardTrackerShow(const rc_client_event_t *event);
    void _OnLeaderboardTrackerHide(const rc_client_event_t *event);
    void _OnLeaderboardTrackerUpdate(const rc_client_event_t *event);
    void _OnLeaderboardScoreboard(const rc_client_event_t *event);
    void _OnReset(const rc_client_event_t *event);
    void _OnGameCompleted(const rc_client_event_t *event);
    void _OnServerError(const rc_client_event_t *event);
    void _OnDisconnected(const rc_client_event_t *event);
    void _OnReconnected(const rc_client_event_t *event);
    void _OnSubsetCompleted(const rc_client_event_t *event);

    rc_client_t *_client;
    bool _online;
    std::map<uint32_t, Achievement *> _achievements;
    uint32_t _game_id;
    retro_memory_map _mmap;
    RetroMemory *_retro_memory;
    TextureCache _texture_cache;
    int _login_session_count;
};