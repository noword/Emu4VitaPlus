#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "app.h"
#include "log.h"
#include "defines.h"
#include "global.h"

#define SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT (0xffffffffU)
unsigned int sceLibcHeapExtendedAlloc __attribute__((used)) = 1;
unsigned int sceLibcHeapSize __attribute__((used)) = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;
unsigned int sceUserMainThreadStackSize __attribute__((used)) = 0x1000000; // 1M

extern "C"
{
    extern unsigned _newlib_heap_size;
    extern char *_newlib_heap_end, *_newlib_heap_cur;
}

static void ParseParams(int argc, char *const argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--arch") == 0)
        {
            gBootFromArch = true;
        }
        else if (strcmp(argv[i], "--rom") == 0)
        {
            i++;
            if (File::Exist(argv[i]))
                gBootRomPath = argv[i];
        }
    }
}

#define LOG_STR_DEFINE(X) LogDebug("%20s : %s", #X, X);
#define LOG_DEFINE(X) LogDebug("%20s : %d", #X, X);

static void LogDefines()
{
    LOG_STR_DEFINE(CONSOLE_DIR);
    LOG_STR_DEFINE(CORE_DATA_DIR);
    LOG_STR_DEFINE(CORE_SAVEFILES_DIR);
    LOG_STR_DEFINE(CORE_CHEATS_DIR);
    LOG_STR_DEFINE(CORE_LOG_PATH);
    LOG_STR_DEFINE(CORE_CONFIG_PATH);
    LOG_STR_DEFINE(CORE_INPUT_DESC_PATH);
    LOG_STR_DEFINE(CORE_FAVOURITE_PATH);

    LOG_DEFINE(DEFAULT_ENABLE_REWIND);
    LOG_DEFINE(DEFAULT_REWIND_BUF_SIZE);
    LOG_DEFINE(DEFAULT_AUTO_SAVE);
    LOG_DEFINE(DEFAULT_REBOOT_WHEN_LOADING_AGAIN);

    LOG_DEFINE(DEFAULT_INDEPENDENT_CONFIG);
    LOG_DEFINE(DEFAULT_LIGHTGUN);
    LOG_DEFINE(DEFAULT_MOUSE);
    LOG_DEFINE(ENABLE_KEYBOARD);
    LOG_DEFINE(CONTROL_SPEED_BY_VIDEO);
}

int main(int argc, char *const argv[])
{
    ParseParams(argc, argv);

    File::MakeDirs(CORE_DATA_DIR);
    File::MakeDirs(CORE_SYSTEM_DIR);
    if (THUMBNAILS_PATH[0])
    {
        File::MakeDirs(THUMBNAILS_PATH);
    }

    gLog = new Log(CORE_LOG_PATH);

    LogInfo("Emu4Vita++ v%s", APP_VER_STR);
    LogInfo("updated on " __DATE__ " " __TIME__);

    LogDefines();

    // must use for keeping this variables
    // LogInfo("%d", _newlib_heap_size_user);
    LogInfo("%d", sceUserMainThreadStackSize);
    LogInfo("%d", sceLibcHeapExtendedAlloc);
    LogInfo("%d", sceLibcHeapSize);

    LogDebug("stack size: %d", sceKernelCheckThreadStack());

    {
        App app;
        LogInfo("_newlib_heap_size: %d", _newlib_heap_size);
        LogInfo("free heap: %d", _newlib_heap_end - _newlib_heap_cur);
        app.Run();
    }

    LogInfo("Exit main()");

    delete gLog;

    return 0;
}
