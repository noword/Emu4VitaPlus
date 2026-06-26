#include <psp2/kernel/processmgr.h>
#include <psp2/apputil.h>
#include <psp2/appmgr.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "log.h"
#include "app.h"
#include "defines.h"
#include "config.h"

// Adds RetroArch-style boot-URI launching. External launchers
// can do:
//     sceAppMgrLaunchAppByUri(0xFFFFF,
//         "psgm:play?titleid=EMU4VPLUS&core=<core_name>&rom=<ux0_path>");
// On startup we read the URI via sceAppMgrGetAppParam, set gCoreName from
// &core=, stash the ROM path from &rom=, skip the arch UI, and forward
// "--rom <path>" in the existing chain-load argv to the per-core eboot.
// The per-core eboot's existing ParseParams() then auto-loads the ROM.

static char gUriRomPath[1024] = {0};

static bool IsHex(char c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static int HexValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

static void UrlDecodeInPlace(char *s)
{
    char *r = s, *w = s;
    while (*r)
    {
        if (*r == '%' && IsHex(r[1]) && IsHex(r[2]))
        {
            *w++ = (char)((HexValue(r[1]) << 4) | HexValue(r[2]));
            r += 3;
        }
        else if (*r == '+')
        {
            *w++ = ' ';
            r++;
        }
        else
        {
            *w++ = *r++;
        }
    }
    *w = 0;
}

static bool ExtractParam(const char *uri, const char *key, char *out, size_t out_sz)
{
    char needle[64];
    snprintf(needle, sizeof(needle), "&%s=", key);
    const char *p = strstr(uri, needle);
    if (!p)
    {
        snprintf(needle, sizeof(needle), "?%s=", key);
        p = strstr(uri, needle);
        if (!p)
            return false;
    }
    p += strlen(needle);
    const char *end = strchr(p, '&');
    size_t len = end ? (size_t)(end - p) : strlen(p);
    if (len >= out_sz)
        len = out_sz - 1;
    memcpy(out, p, len);
    out[len] = 0;
    UrlDecodeInPlace(out);
    return out[0] != 0;
}

static bool IsSafeCoreName(const char *core)
{
    for (const char *p = core; *p; ++p)
    {
        bool ok = (*p >= 'a' && *p <= 'z') ||
                  (*p >= 'A' && *p <= 'Z') ||
                  (*p >= '0' && *p <= '9') ||
                  *p == '_';
        if (!ok)
            return false;
    }
    return core[0] != 0;
}

// Returns true iff URI provided a usable &core= (with optional &rom=).
static bool ParseBootUri()
{
    SceAppUtilInitParam init_param;
    SceAppUtilBootParam boot_param;
    memset(&init_param, 0, sizeof(init_param));
    memset(&boot_param, 0, sizeof(boot_param));
    sceAppUtilInit(&init_param, &boot_param);

    char uri[2048] = {0};
    int r = sceAppMgrGetAppParam(uri);
    if (r != 0 || uri[0] == 0)
    {
        sceAppUtilShutdown();
        return false;
    }
    LogInfo("Boot URI: %s", uri);

    char core[64] = {0};
    if (!ExtractParam(uri, "core", core, sizeof(core)) || !IsSafeCoreName(core))
    {
        sceAppUtilShutdown();
        return false;
    }
    gCoreName = core;

    ExtractParam(uri, "rom", gUriRomPath, sizeof(gUriRomPath));
    LogInfo("URI core=%s rom=%s", core, gUriRomPath);

    sceAppUtilShutdown();
    return true;
}

int main(int argc, char *const argv[])
{
    File::MakeDirs(APP_DATA_DIR);
    gLog = new Log(APP_LOG_PATH);

    LogInfo("updated on " __DATE__ " " __TIME__);
    LogDebug("Start");

    gConfig = new Config();

    bool skipUi = ParseBootUri();
    if (!skipUi)
    {
        App *app = new App();
        app->Run();
        delete app;
    }

    LogDebug("Exit");

    if (gCoreName.size() > 0)
    {
        LogInfo("gCoreName: %s", gCoreName.c_str());
        const char *_argv[6];
        int ai = 0;
        _argv[ai++] = "--arch";
        if (gUriRomPath[0])
        {
            _argv[ai++] = "--rom";
            _argv[ai++] = gUriRomPath;
        }
        _argv[ai] = NULL;

        char app_path[SCE_FIOS_PATH_MAX];
        snprintf(app_path, SCE_FIOS_PATH_MAX, "app0:eboot_%s.self", gCoreName.c_str());
        SceInt32 result = sceAppMgrLoadExec(app_path, (char *const *)_argv, NULL);
        if (result != SCE_OK)
        {
            LogError("sceAppMgrLoadExec failed: %08x", result);
        }
    }

    delete gLog;

    return 0;
}
