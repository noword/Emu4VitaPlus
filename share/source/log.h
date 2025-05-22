#pragma once

#include <stdio.h>
#include <string>
#include <psp2/kernel/processmgr.h>

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5

#define LOG_LEVEL_OFF 6

const char LogLevelChars[] = "TDIWEFO";

#ifndef LOG_LEVEL
#ifdef DEBUG
#define LOG_LEVEL LOG_LEVEL_DEBUG
#else
#define LOG_LEVEL LOG_LEVEL_INFO
#endif
#endif

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LogTrace(fmt, ...) gLog->log(LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#else
#define LogTrace(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LogDebug(fmt, ...) gLog->log(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#else
#define LogDebug(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LogInfo(fmt, ...) gLog->log(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#else
#define LogInfo(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LogWarn(fmt, ...) gLog->log(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#else
#define LogWarn(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LogError(fmt, ...) gLog->log(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#else
#define LogError(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_FATAL
#define LogFatal(fmt, ...) gLog->log(LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#else
#define LogFatal(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LogFunctionName LogDebug(__PRETTY_FUNCTION__);

#define LogFunctionNameLimited                                     \
    {                                                              \
        static int LOG_COUNT = 10;                                 \
        if (LOG_COUNT >= 0)                                        \
        {                                                          \
            LogDebug("%s (%d)", __PRETTY_FUNCTION__, LOG_COUNT--); \
        }                                                          \
    }

#define LogInfoLimited(fmt, ...)                              \
    {                                                         \
        static int LOG_COUNT = 10;                            \
        if (LOG_COUNT >= 0)                                   \
        {                                                     \
            LogInfo(fmt " (%d)", ##__VA_ARGS__, LOG_COUNT--); \
        }                                                     \
    }

#ifdef __cplusplus
extern "C"
{
#endif

    void CLogTrace(const char *fmt, ...);
    void CLogDebug(const char *fmt, ...);
    void CLogInfo(const char *fmt, ...);
    void CLogWarn(const char *fmt, ...);
    void CLogError(const char *fmt, ...);
    void CLogFatal(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#else
#define LogFunctionName
#define LogFunctionNameLimited
#define LogInfoLimited(fmt, ...)
#endif

class Log
{
public:
    Log(const char *name, int buf_len = 2048);
    virtual ~Log();
    void log(int log_level, const char *format, ...);
    void log_v(int log_level, const char *format, va_list args);

private:
    std::string _name;
    char *_buf;
    int _buf_len;
    SceKernelLwMutexWork _mutex;
    FILE *_fp;
};

#if LOG_LEVEL != LOG_LEVEL_OFF
extern Log *gLog;
#endif