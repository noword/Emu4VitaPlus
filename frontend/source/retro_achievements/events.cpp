#include <psp2/rtc.h>
#include "retro_achievements.h"
#include "language_string.h"
#include "global.h"
#include "log.h"

#define PROGRESS_INDICATOR_ID 0xffffffff

RetroAchievements::EventFunc RetroAchievements::_event_functions[] = {
    nullptr,
    &RetroAchievements::_OnAchievementTriggered,
    &RetroAchievements::_OnLeaderboardStarted,
    &RetroAchievements::_OnLeaderboardFailed,
    &RetroAchievements::_OnLeaderboardSubmitted,
    &RetroAchievements::_OnAchievementChallengeIndicatorShow,
    &RetroAchievements::_OnAchievementChallengeIndicatorHide,
    &RetroAchievements::_OnAchievementProgressIndicatorShow,
    &RetroAchievements::_OnAchievementProgressIndicatorHide,
    &RetroAchievements::_OnAchievementProgressIndicatorUpdate,
    &RetroAchievements::_OnLeaderboardTrackerShow,
    &RetroAchievements::_OnLeaderboardTrackerHide,
    &RetroAchievements::_OnLeaderboardTrackerUpdate,
    &RetroAchievements::_OnLeaderboardScoreboard,
    &RetroAchievements::_OnReset,
    &RetroAchievements::_OnGameCompleted,
    &RetroAchievements::_OnServerError,
    &RetroAchievements::_OnDisconnected,
    &RetroAchievements::_OnReconnected,
    &RetroAchievements::_OnSubsetCompleted};

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

void RetroAchievements::_OnAchievementTriggered(const rc_client_event_t *event)
{
    LogFunctionName;
    const auto achievement = event->achievement;
    Notification *notification = new Notification;
    notification->title = TEXT(LANG_ACHIEVEMENT_UNLOCKED);
    notification->text = achievement->title;

    auto iter = _achievements.find(achievement->id);
    if (iter == _achievements.end())
    {
        char url[128];
        if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, url, sizeof(url)) == RC_OK)
        {
            notification->texture = _texture_cache.Get(url, _game_id, achievement->id);
        }
    }
    else
    {
        iter->second->SetState(true);
        SceDateTime time;
        sceRtcGetCurrentClockLocalTime(&time);
        char buf[32];
        snprintf(buf, 32, "%d-%d-%d %02d:%02d:%02d", time.year, time.month, time.day, time.hour, time.minute, time.second);
        iter->second->unlock_time = buf;
        notification->texture = vita2d_load_PNG_file(iter->second->GetImagePath(true).c_str());
    }

    notification->SetShowTime();
    gNotifications->Add(achievement->id, notification);
}

void RetroAchievements::_OnLeaderboardStarted(const rc_client_event_t *event)
{
    LogFunctionName;
    auto leaderboard = event->leaderboard;
    Notification *notification = new Notification;
    notification->title = TEXT(LANG_LEADERBOARD_STARTED);
    notification->text = leaderboard->title;
    notification->SetShowTime();
    gNotifications->Add(leaderboard->id, notification);
}

void RetroAchievements::_OnLeaderboardFailed(const rc_client_event_t *event)
{
    LogFunctionName;
    auto leaderboard = event->leaderboard;
    Notification *notification = new Notification;
    notification->title = TEXT(LANG_LEADERBOARD_FAILED);
    notification->text = leaderboard->title;
    notification->SetShowTime();
    gNotifications->Add(leaderboard->id, notification);
}

void RetroAchievements::_OnLeaderboardSubmitted(const rc_client_event_t *event)
{
    LogFunctionName;

    auto leaderboard = event->leaderboard;
    Notification *notification = new Notification;
    notification->title = TEXT(LANG_LEADERBOARD_SUBMITTED);
    notification->text = leaderboard->title;
    notification->SetShowTime();
    gNotifications->Add(leaderboard->id, notification);
}

void RetroAchievements::_OnAchievementChallengeIndicatorShow(const rc_client_event_t *event)
{
    LogFunctionName;

    auto achievement = event->achievement;
    Notification *notification = new Notification;
    char url[128];
    if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, url, sizeof(url)) == RC_OK)
    {
        notification->texture = _texture_cache.Get(url, _game_id, achievement->id);
    }
    gNotifications->Add(achievement->id, notification);
}

void RetroAchievements::_OnAchievementChallengeIndicatorHide(const rc_client_event_t *event)
{
    LogFunctionName;
    gNotifications->Remove(event->achievement->id);
}

void RetroAchievements::_OnAchievementProgressIndicatorShow(const rc_client_event_t *event)
{
    LogFunctionName;

    auto achievement = event->achievement;
    Notification *notification = new Notification;
    notification->title = achievement->measured_progress;
    notification->text = achievement->title;
    char url[128];
    if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, url, sizeof(url)) == RC_OK)
    {
        notification->texture = _texture_cache.Get(url, _game_id, achievement->id);
    }
    gNotifications->Add(PROGRESS_INDICATOR_ID, notification);
}

void RetroAchievements::_OnAchievementProgressIndicatorHide(const rc_client_event_t *event)
{
    LogFunctionName;

    gNotifications->Remove(PROGRESS_INDICATOR_ID);
}

void RetroAchievements::_OnAchievementProgressIndicatorUpdate(const rc_client_event_t *event)
{
    LogFunctionName;

    auto achievement = event->achievement;
    gNotifications->Update(PROGRESS_INDICATOR_ID, achievement->measured_progress);
}

void RetroAchievements::_OnLeaderboardTrackerShow(const rc_client_event_t *event)
{
    LogFunctionName;

    auto tracker = event->leaderboard_tracker;
    Notification *notification = new Notification;
    notification->title = tracker->display;
    gNotifications->Add(tracker->id, notification);
}

void RetroAchievements::_OnLeaderboardTrackerHide(const rc_client_event_t *event)
{
    LogFunctionName;

    gNotifications->Remove(event->leaderboard_tracker->id);
}

void RetroAchievements::_OnLeaderboardTrackerUpdate(const rc_client_event_t *event)
{
    LogFunctionName;

    auto tracker = event->leaderboard_tracker;
    gNotifications->Update(tracker->id, tracker->display);
}

void RetroAchievements::_OnLeaderboardScoreboard(const rc_client_event_t *event)
{
    LogFunctionName;
    auto leaderboard = event->leaderboard;
    auto scoreboard = event->leaderboard_scoreboard;
    Notification *notification = new Notification;

    notification->title = leaderboard->title;
    if (scoreboard)
    {
        notification->text = scoreboard->submitted_score;
    }
    else
    {
        notification->text = leaderboard->tracker_value;
    }

    notification->SetShowTime();
    gNotifications->Add(leaderboard->id, notification);
}

void RetroAchievements::_OnReset(const rc_client_event_t *event)
{
    LogFunctionName;
    gEmulator->Reset();
}

void RetroAchievements::_OnGameCompleted(const rc_client_event_t *event)
{
    LogFunctionName;

    const rc_client_game_t *game = rc_client_get_game_info(_client);
    Notification *notification = new Notification;
    notification->title = TEXT(LANG_GAME_COMPLETED);
    notification->text = game->title;

    char url[128];
    if (rc_client_game_get_image_url(game, url, sizeof(url)) == RC_OK)
    {
        notification->texture = _texture_cache.Get(url, _game_id, GAME_IMAGE_ID);
    }
    notification->SetShowTime();
    gNotifications->Add(game->id, notification);
}

void RetroAchievements::_OnServerError(const rc_client_event_t *event) { LogFunctionName; }

void RetroAchievements::_OnDisconnected(const rc_client_event_t *event)
{
    LogFunctionName;
    _online = false;
}

void RetroAchievements::_OnReconnected(const rc_client_event_t *event)
{
    LogFunctionName;
    _online = true;
}

void RetroAchievements::_OnSubsetCompleted(const rc_client_event_t *event) { LogFunctionName; }