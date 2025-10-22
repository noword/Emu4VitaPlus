#include <psp2/rtc.h>
#include "retro_achievements.h"
#include "language_string.h"
#include "log.h"

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
            notification->texture = _GetImage(url, achievement->id);
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
    AddNotification(achievement->id, notification);
}

void RetroAchievements::_OnLeaderboardStarted(const rc_client_event_t *event)
{
    LogFunctionName;
    auto leaderboard = event->leaderboard;
    Notification *notification = new Notification;
    notification->title = TEXT(LANG_LEADERBOARD_STARTED);
    notification->text = leaderboard->title;
    notification->SetShowTime();
    AddNotification(leaderboard->id, notification);
}

void RetroAchievements::_OnLeaderboardFailed(const rc_client_event_t *event)
{
    LogFunctionName;
    auto leaderboard = event->leaderboard;
    Notification *notification = new Notification;
    notification->title = TEXT(LANG_LEADERBOARD_FAILED);
    notification->text = leaderboard->title;
    notification->SetShowTime();
    AddNotification(leaderboard->id, notification);
}

void RetroAchievements::_OnLeaderboardSubmitted(const rc_client_event_t *event)
{
    LogFunctionName;

    auto leaderboard = event->leaderboard;
    Notification *notification = new Notification;
    notification->title = TEXT(LANG_LEADERBOARD_SUBMITTED);
    notification->text = leaderboard->title;
    notification->SetShowTime();
    AddNotification(leaderboard->id, notification);
}

void RetroAchievements::_OnAchievementChallengeIndicatorShow(const rc_client_event_t *event)
{
    LogFunctionName;

    auto achievement = event->achievement;
    Notification *notification = new Notification;
    char url[128];
    if (rc_client_achievement_get_image_url(achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, url, sizeof(url)) == RC_OK)
    {
        notification->texture = _GetImage(url, achievement->id);
    }
    AddNotification(achievement->id, notification);
}

void RetroAchievements::_OnAchievementChallengeIndicatorHide(const rc_client_event_t *event)
{
    LogFunctionName;
    RemoveNotification(event->achievement->id);
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
        notification->texture = _GetImage(url, achievement->id);
    }
    AddNotification(achievement->id, notification);
}

void RetroAchievements::_OnAchievementProgressIndicatorHide(const rc_client_event_t *event)
{
    LogFunctionName;

    RemoveNotification(event->achievement->id);
}

void RetroAchievements::_OnAchievementProgressIndicatorUpdate(const rc_client_event_t *event)
{
    LogFunctionName;

    auto achievement = event->achievement;
    UpdateNotification(achievement->id, achievement->measured_progress);
}

void RetroAchievements::_OnLeaderboardTrackerShow(const rc_client_event_t *event)
{
    LogFunctionName;

    auto tracker = event->leaderboard_tracker;
    Notification *notification = new Notification;
    notification->title = tracker->display;
    AddNotification(tracker->id, notification);
}

void RetroAchievements::_OnLeaderboardTrackerHide(const rc_client_event_t *event)
{
    LogFunctionName;

    RemoveNotification(event->leaderboard_tracker->id);
}

void RetroAchievements::_OnLeaderboardTrackerUpdate(const rc_client_event_t *event)
{
    LogFunctionName;

    auto tracker = event->leaderboard_tracker;
    UpdateNotification(tracker->id, tracker->display);
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
    AddNotification(leaderboard->id, notification);
}

void RetroAchievements::_OnReset(const rc_client_event_t *event) { LogFunctionName; }

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
        notification->texture = _GetImage(url, GAME_IMAGE_ID);
    }
    notification->SetShowTime();
    AddNotification(game->id, notification);
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