#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "app.h"
#include "log.h"
#include "profiler.h"
#include "defines.h"
#include "retro_module.h"
#include "cores.h"
#include "global.h"

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

    RetroModule *module = new RetroModule(gCore->core_name);

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
