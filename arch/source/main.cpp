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

    gConfig = new Config();

    App *app = new App();
    app->Run();
    delete app;

    LogDebug("Exit");

    if (gCoreName.size() > 0)
    {
        LogInfo("gCoreName: %s", gCoreName.c_str());
        const char *const _argv[] = {"--arch", NULL};
        char app_path[SCE_FIOS_PATH_MAX];
        snprintf(app_path, SCE_FIOS_PATH_MAX, "app0:cores/eboot_%s.self", gCoreName.c_str());
        SceInt32 result = sceAppMgrLoadExec(app_path, (char *const *)_argv, NULL);
        if (result != SCE_OK)
        {
            LogError("sceAppMgrLoadExec failed: %08x", result);
        }
    }

    delete gLog;

    return 0;
}
