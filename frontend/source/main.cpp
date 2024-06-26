#include <stdlib.h>
#include "file.h"
#include "app.h"
#include "log.h"
#include "defines.h"

#define SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT (0xffffffffU)

unsigned int sceLibcHeapExtendedAlloc = 1;
unsigned int sceLibcHeapSize = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT;

int main(int argc, char *const argv[])
{
    File::MakeDirs(APP_DATA_DIR);
    File::MakeDirs(CORE_SYSTEM_DIR);
    gLog = new Log(APP_LOG_PATH);
    LogInfo("updated on " __DATE__ " " __TIME__);
    LogDebug("Start");
    {
        App app;
        app.Run();
    }

    LogDebug("Exit");

    delete gLog;

    sceKernelExitProcess(0);
    return 0;
}
