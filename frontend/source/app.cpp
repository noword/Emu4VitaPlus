#include <psp2/ctrl.h>
#include <psp2/power.h>
#include <psp2/touch.h>
#include <psp2/apputil.h>
#include <psp2/display.h>
#include <vita2d.h>
#include <jsoncpp/json/json.h>
#include "my_imgui.h"
#include "app.h"
#include "config.h"
#include "state_manager.h"
#include "log.h"
#include "defines.h"
#include "shader.h"
#include "favorite.h"
#include "archive_reader_factory.h"
#include "input_descriptor.h"
#include "bios_checker.h"
#include "language_string.h"
#include "global.h"
#include "profiler.h"
#include "Network.h"

#ifdef TEXT
#undef TEXT
#endif

using namespace Emu4VitaPlus;

void OnVersionChecked(bool has_new)
{
    LogFunctionName;

    if (has_new)
    {
        gUi->SetHint(TEXT(LANG_NEW_VERSION_AVAILABLE), 3 * 60);
    }
}

App::App()
{
    LogFunctionName;

    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    sceShellUtilInitEvents(0);

    SceAppUtilInitParam init_param{0};
    SceAppUtilBootParam boot_param{0};
    sceAppUtilInit(&init_param, &boot_param);

    SceCommonDialogConfigParam config;
    sceCommonDialogConfigParamInit(&config);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, (int *)&config.language);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, (int *)&config.enterButtonAssign);
    sceCommonDialogSetConfigParam(&config);

    sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG);

    // sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    // sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
    gProfiler = new Profiler();
#endif

    _IsSaveMode();
    // LogDebug("getVMBlock: %08x", getVMBlock());
    gConfig = new Config();

    if (!gConfig->Load())
    {
        File::RemoveAllFiles(ARCADE_CACHE_DIR);
        File::RemoveAllFiles(ARCHIVE_CACHE_DIR);
        File::RemoveAllFiles(PLAYLISTS_CACHE_DIR);
        File::RemoveAllFiles(CACHE_DIR);
        gConfig->Save();
    }

    SwapEnterButton(gConfig->swap_enter);

    Network::Init();

    gEmulator = new Emulator();
    gVideo = new Video();
    gUi = new Ui();
    gVideo->Start();

    gUi->AppendLog("Booting");
    gUi->AppendLog("Parse params");
    gUi->AppendLog("Initialize video");
    gUi->AppendLog("Initialize core spec settings");
    gUi->AppendLog("Initialize network");

    gUi->AppendLog("Initialize emulator");
    gEmulator->Init();

    gUi->AppendLog("Initialize state manager");
    gStateManager = new CoreStateManager();

    gUi->AppendLog("Initialize archive reader factory");
    gArchiveReaderFactory = new ArchiveReaderFactory();

    gUi->AppendLog("Load overlays");
    gOverlays = new Overlays;

    gUi->AppendLog("Load shaders");
    gShaders = new Shaders;

    gUi->AppendLog("Load Favorites");
    gFavorites = new Favorites;

    gUi->AppendLog("Load RetroArch Playlists");
    gPlaylists = new RetroArchPlaylists;
    gPlaylists->LoadAll();

    gUi->AppendLog("Load Rom names DB");
    gRomNameMap = new RomNameMap;
    gRomNameMap->Load();

    gUi->AppendLog("Create tables of UI");
    gUi->CreateTables();

    if (gConfig->language != LANGUAGE::LANGUAGE_ENGLISH)
    {
        gUi->AppendLog("Load font");
        gVideo->Lock();
        My_Imgui_Destroy_Font();
        My_Imgui_Create_Font(gConfig->language, CACHE_DIR);
        gVideo->Unlock();
    }

    gUi->AppendLog("Check BIOS files");
    std::vector<const char *> bios;
    if (!CheckBios(bios))
    {
        std::string bios_hint = TEXT(LANG_MISSING_BIOS);
        char tmp[SCE_FIOS_PATH_MAX];
        for (const auto &name : bios)
        {
            snprintf(tmp, SCE_FIOS_PATH_MAX, "missing BIOS file: %s", name);
            gUi->AppendLog(tmp);
            bios_hint += "\n";
            bios_hint += tmp;
        }

        gUi->SetHint(bios_hint.c_str(), 5 * 60);
    }

    gUi->ClearLogs();

    Utils::CheckVersion(OnVersionChecked);
}

App::~App()
{
    LogFunctionName;

    gVideo->Stop();
    vita2d_wait_rendering_done();

    delete gRomNameMap;
    delete gStateManager;
    delete gShaders;
    delete gOverlays;
    delete gArchiveReaderFactory;
    delete gEmulator;
    delete gUi;
    delete gVideo;
    delete gConfig;

    Network::Deinit();

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
    delete gProfiler;
#endif

    sceAppUtilShutdown();
}

void App::Run()
{
    LogFunctionName;
    bool running = true;
    APP_STATUS last_status;

    if (gBootRomPath.empty())
    {
        last_status = APP_STATUS_SHOW_UI;
        gStatus.Set(last_status);
    }
    else
    {
        last_status = APP_STATUS_BOOT;
        gEmulator->LoadRom(gBootRomPath.c_str(), NULL, 0);
    }

    while (running)
    {
        APP_STATUS status = gStatus.Get();
        if (status != last_status)
        {
            gUi->OnStatusChanged(status);
            last_status = status;
        }

        switch (gStatus.Get())
        {
        case APP_STATUS_BOOT:
        case APP_STATUS_SHOW_UI:
        case APP_STATUS_SHOW_UI_IN_GAME:
            gUi->Run();
            break;

        case APP_STATUS_RUN_GAME:
        case APP_STATUS_REWIND_GAME:
            gEmulator->Run();
            break;

        case APP_STATUS_RETURN_ARCH:
        {
            char *argv[] = {NULL};
            sceAppMgrLoadExec("app0:eboot.bin", argv, NULL);
            gStatus.Set(APP_STATUS_EXIT);
        }
        break;

        case APP_STATUS_REBOOT_WITH_LOADING:
        {
            char boot[SCE_FIOS_PATH_MAX];
            const char *argv[] = {"", "--rom", gEmulator->GetCurrentName(), NULL};
            if (gBootFromArch)
            {
                snprintf(boot, SCE_FIOS_PATH_MAX, "app0:eboot_%s.self", CORE_NAME);
                argv[0] = "--arch";
            }
            else
            {
                strcpy(boot, "app0:eboot.bin");
            }
            sceAppMgrLoadExec(boot, (char *const *)argv, NULL);
            gStatus.Set(APP_STATUS_EXIT);
        }
        break;

        case APP_STATUS_EXIT:
            running = false;
            break;

        default:
            break;
        }
    }
}

bool App::_IsSaveMode()
{
    return sceIoDevctl("ux0:", 0x3001, NULL, 0, NULL, 0) == 0x80010030;
}
