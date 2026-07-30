#include <psp2/ctrl.h>
#include <psp2/shellutil.h>
#include "escape_thread.h"
#include "log.h"

// 1 second
#define HOLDING_CHECK_INTERVAL 1000000
// 4 seconds
#define PS_HOLDING_TIME 4000000

int EscapeThread::_EscapeThread(SceSize args, void *argp)
{
    LogFunctionName;

    CLASS_POINTER(EscapeThread, escape, argp);

    uint64_t ps_holding_start = 0ll;
    Delay<uint64_t> delay{HOLDING_CHECK_INTERVAL};

    while (escape->IsRunning())
    {
        SceCtrlData ctrl_data{0};
        sceCtrlReadBufferPositiveExt2(0, &ctrl_data, 1);
        if ((ctrl_data.buttons & SCE_CTRL_PSBUTTON) == SCE_CTRL_PSBUTTON)
        {
            if (ps_holding_start == 0ll)
            {
                ps_holding_start = sceKernelGetProcessTimeWide();
            }
            else if (sceKernelGetProcessTimeWide() - ps_holding_start >= PS_HOLDING_TIME)
            {
                ps_holding_start = 0ll;
                LogInfo("Unlock PS button");
                sceShellUtilUnlock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN |
                                                          SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU |
                                                          SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION |
                                                          SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2));
            }
        }
        else if (ps_holding_start != 0ll)
        {
            ps_holding_start = 0ll;
        }

        delay.Wait();
    }

    LogDebug("_EscapeThread exit");

    return sceKernelExitThread(0);
}