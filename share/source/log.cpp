#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include <psp2common/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include "file.h"
#include "log.h"

#if LOG_LEVEL != LOG_LEVEL_OFF
Log *gLog = NULL;
#if LOG_LEVEL > LOG_LEVEL_DEBUG
#define LOG_QUICK
#endif

#endif

Log::Log(const char *name, int buf_len)
{
	File::Remove(name);
	_name = name;
	_buf = new char[buf_len];
	_buf_len = buf_len;
#ifdef LOG_QUICK
	_fp = fopen(name, "w");
#endif
}

Log::~Log()
{
	delete[] _buf;

#ifdef LOG_QUICK
	fclose(_fp);
#endif
}

void Log::log(int log_level, const char *format, ...)
{

	va_list args;
	va_start(args, format);
	log_v(log_level, format, args);
	va_end(args);
}

void Log::log_v(int log_level, const char *format, va_list args)
{
	_locker.Lock();
#ifndef LOG_QUICK
	_fp = fopen(_name.c_str(), "a");
#endif
	if (_fp)
	{
		vsnprintf(_buf, _buf_len, format, args);
		SceDateTime time;
		sceRtcGetCurrentClockLocalTime(&time);
		fprintf(_fp, "[%c] %02d:%02d:%02d.%03d %s\n", LogLevelChars[log_level], time.hour, time.minute, time.second, time.microsecond / 1000, _buf);
		sceClibPrintf("[%c] %02d:%02d:%02d.%03d %s\n", LogLevelChars[log_level], time.hour, time.minute, time.second, time.microsecond / 1000, _buf);
#ifndef LOG_QUICK
		fclose(_fp);
#else
		fflush(_fp);
#endif
	}
	_locker.Unlock();
}

#ifdef __cplusplus
extern "C"
{
#endif

#define CLOG_INTERFACE(FORMAT, LEVEL)     \
	{                                     \
		va_list args;                     \
		va_start(args, FORMAT);           \
		gLog->log_v(LEVEL, FORMAT, args); \
		va_end(args);                     \
	}

	void CLogTrace(const char *fmt, ...)
	{
		CLOG_INTERFACE(fmt, LOG_LEVEL_TRACE);
	}

	void CLogDebug(const char *fmt, ...)
	{
		CLOG_INTERFACE(fmt, LOG_LEVEL_DEBUG);
	}

	void CLogInfo(const char *fmt, ...)
	{
		CLOG_INTERFACE(fmt, LOG_LEVEL_INFO);
	}

	void CLogWarn(const char *fmt, ...)
	{
		CLOG_INTERFACE(fmt, LOG_LEVEL_WARN);
	}

	void CLogError(const char *fmt, ...)
	{
		CLOG_INTERFACE(fmt, LOG_LEVEL_ERROR);
	}

	void CLogFatal(const char *fmt, ...)
	{
		CLOG_INTERFACE(fmt, LOG_LEVEL_FATAL);
	}

#ifdef __cplusplus
}
#endif