#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <psp2/io/fcntl.h>
#include <psp2/rtc.h>
#include <psp2common/kernel/threadmgr.h>
#include "file.h"
#include "log.h"

#if LOG_LEVEL != LOG_LEVEL_OFF
Log *gLog = NULL;
#endif

Log::Log(const char *name, int buf_len)
{
	File::Remove(name);
	_fp = fopen(name, "w");
	_buf = new char[buf_len];
	_buf_len = buf_len;
	sceKernelCreateLwMutex(&_mutex, "log_mutex", 0, 0, NULL);
}

Log::~Log()
{
	sceKernelLockLwMutex(&_mutex, 1, NULL);
	delete[] _buf;
	sceKernelDeleteLwMutex(&_mutex);

	if (_fp)
		fclose(_fp);
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
	sceKernelLockLwMutex(&_mutex, 1, NULL);
	if (_fp)
	{
		vsnprintf(_buf, _buf_len, format, args);
		SceDateTime time;
		sceRtcGetCurrentClockLocalTime(&time);
		fprintf(_fp, "[%c] %02d:%02d:%02d.%03d %s\n", LogLevelChars[log_level], time.hour, time.minute, time.second, time.microsecond / 1000, _buf);
		fflush(_fp);
	}
	sceKernelUnlockLwMutex(&_mutex, 1);
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