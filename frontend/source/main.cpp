#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "app.h"
#include "log.h"
#include "profiler.h"
#include "defines.h"
#include "retro_module.h"
#include "cores.h"

#define SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT (0xffffffffU)

unsigned int sceUserMainThreadStackSize __attribute__((used)) = 0x100000;
unsigned int sceLibcHeapExtendedAlloc __attribute__((used)) = 1;
unsigned int sceLibcHeapSize __attribute__((used)) = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;

static void ParseParams(int argc, char *const argv[])
{
    for (int i = 0; i < argc; i++)
    {
        // LogDebug("argv[%d]: %s", i, argv[i]);
        if (strcmp(argv[i], "--arch") == 0)
        {
            gBootFromArch = true;
            // gConfig->boot_from_arch = true;
        }
        else if (strcmp(argv[i], "--core") == 0)
        {
            i++;
            strcpy(CORE_NAME, argv[i]);
        }
        else if (strcmp(argv[i], "--rom") == 0)
        {
            i++;
            gBootRomPath = argv[i];
        }
    }
}

static bool InitDefines()
{
    for (const auto &c : CORES)
    {
        if (strcmp(c.core_name, CORE_NAME) == 0)
        {
            gCore = &c;
            break;
        }
    }

    if (gCore == nullptr)
    {
        return false;
    }

    snprintf(CONSOLE_DIR, DEFINE_PATH_LENGTH, "app0:data/", gCore->console_name);

    snprintf(CORE_DATA_DIR, DEFINE_PATH_LENGTH, ROOT_DIR "/%s", CORE_NAME);
    snprintf(CORE_SAVEFILES_DIR, DEFINE_PATH_LENGTH, "%s/savefiles ", CORE_DATA_DIR);
    snprintf(CORE_CHEATS_DIR, DEFINE_PATH_LENGTH, "%s/cheats ", CORE_DATA_DIR);

    snprintf(CORE_LOG_PATH, DEFINE_PATH_LENGTH, "%s//Emu4Vita++.log ", CORE_DATA_DIR);
    snprintf(CORE_CONFIG_PATH, DEFINE_PATH_LENGTH, "%s/config.ini ", CORE_DATA_DIR);
    snprintf(CORE_INPUT_DESC_PATH, DEFINE_PATH_LENGTH, "%s/input_desc.ini ", CORE_DATA_DIR);
    snprintf(CORE_FAVOURITE_PATH, DEFINE_PATH_LENGTH, "%s/favourite.ini ", CORE_DATA_DIR);

    return true;
}

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEFINE(X) LogDebug("%20s : %s", #X, X);
static void LogDefines()
{
    LOG_DEFINE(CONSOLE_DIR);
    LOG_DEFINE(CORE_DATA_DIR);
    LOG_DEFINE(CORE_SAVEFILES_DIR);
    LOG_DEFINE(CORE_CHEATS_DIR);
    LOG_DEFINE(CORE_LOG_PATH);
    LOG_DEFINE(CORE_CONFIG_PATH);
    LOG_DEFINE(CORE_INPUT_DESC_PATH);
    LOG_DEFINE(CORE_FAVOURITE_PATH);
}
#else
#define LogDefines
#endif

int main(int argc, char *const argv[])
{
    ParseParams(argc, argv);
    if (!InitDefines())
    {
        return -1;
    }

    File::MakeDirs(CORE_DATA_DIR);
    File::MakeDirs(CORE_SYSTEM_DIR);
    gLog = new Log(CORE_LOG_PATH);
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
    gProfiler = new Profiler();
#endif
    LogInfo("Emu4Vita++ v%s", APP_VER_STR);
    LogInfo("updated on " __DATE__ " " __TIME__);

    LogDefines();

    RetroModule *module = new RetroModule("app0:gpsp_libretro.suprx");

    // must use for keeping this variables
    LogInfo("%d", sceUserMainThreadStackSize);
    LogInfo("%d", sceLibcHeapExtendedAlloc);
    LogInfo("%d", sceLibcHeapSize);

    LogDebug("stack free size: %d", sceKernelCheckThreadStack());

    {
        App app(argc, argv);
        app.Run();
    }

    delete module;
    LogInfo("Exit main()");

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
    delete gProfiler;
#endif
    delete gLog;

    return 0;
}
