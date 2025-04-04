#include <psp2/kernel/processmgr.h>
#include <stdlib.h>
#include "file.h"
#include "log.h"
#include "app.h"
#include "defines.h"
#include "config.h"

int main(int argc, char *const argv[])
{
    File::MakeDirs(APP_DATA_DIR);
    gLog = new Log(APP_LOG_PATH);

    LogInfo("updated on " __DATE__ " " __TIME__);
    LogDebug("Start");

    App *app = new App();
    app->Run();
    delete app;

    LogDebug("Exit");
    LogInfo("gCorePath: %s", gCorePath);

    if (*gCorePath)
    {
        const char *const _argv[] = {"--arch", NULL};
        SceInt32 result = sceAppMgrLoadExec(gCorePath, (char *const *)_argv, NULL);
        if (result != SCE_OK)
        {
            LogError("sceAppMgrLoadExec failed: %08x", result);
        }
    }

    delete gLog;

    return 0;
}
