#include <functional>
#include <psp2/kernel/threadmgr.h>
#include <vita2d.h>
#include <vita2d_ext.h>
#include "video.h"
#include "global.h"
#include "ui.h"
#include "log.h"
#include "profiler.h"

namespace Emu4VitaPlus
{
    Video::Video() : ThreadBase(_DrawThread)
    {
        LogFunctionName;
        vita2d_init();
        vita2d_ext_init(vita2d_get_context(), vita2d_get_shader_patcher());
        // vita2d_set_vblank_wait(1);
        // vita2d_set_clear_color(0xFF362B00);
    }

    Video::~Video()
    {
        LogFunctionName;
        if (_thread_id >= 0)
        {
            Stop(true);
        }

        vita2d_ext_fini();
        vita2d_fini();
    }

    int Video::_DrawThread(SceSize args, void *argp)
    {
        LogFunctionName;

        CLASS_POINTER(Video, video, argp);

        int vcount = 0;
        APP_STATUS status = gStatus.Get();
        while (video->IsRunning() && (status & (APP_STATUS_EXIT | APP_STATUS_RETURN_ARCH | APP_STATUS_REBOOT_WITH_LOADING)) == 0)
        {
            status = gStatus.Get();
            if (status == APP_STATUS_RUN_GAME && !gEmulator->NeedRender())
            {
                uint32_t timeout = 1000;
                if (video->Wait(&timeout) == SCE_KERNEL_ERROR_WAIT_TIMEOUT)
                {
                    continue;
                }
            }

            video->Lock();

            int old_vcount = vcount;
            vcount = sceDisplayGetVcount();
            vita2d_set_vblank_wait(vcount + 1 >= old_vcount);

            vita2d_pool_reset();
            vita2d_start_drawing_advanced(NULL, 0);
            vita2d_clear_screen();

#ifdef DRAW_IMGUI_TOGETHER
            bool use_imgui = (status & (APP_STATUS_BOOT | APP_STATUS_SHOW_UI | APP_STATUS_SHOW_UI_IN_GAME)) ||
                             gConfig->fps > 0 ||
                             gHint->NeedShow() ||
                             gNotifications->NeedShow();
            if (use_imgui)
            {
                ImGui_ImplVita2D_NewFrame();
                ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            }
#endif

            switch (status)
            {
            case APP_STATUS_BOOT:
            case APP_STATUS_SHOW_UI:
                gUi->Show();
                break;

            case APP_STATUS_RUN_GAME:
            case APP_STATUS_REWIND_GAME:
                // BeginProfile("Video");
                gEmulator->Show();
                // EndProfile("Video");
                break;

            case APP_STATUS_SHOW_UI_IN_GAME:
                gEmulator->Show();
                gUi->Show();
                break;

            default:
                break;
            }

#ifdef DRAW_IMGUI_TOGETHER
            if (use_imgui)
            {
                gHint->Show();
                gNotifications->Show();

                ImGui::Render();
                My_ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
            }
#else
            gHint->Show();
            gNotifications->Show();
#endif
            vita2d_end_drawing();
            vita2d_common_dialog_update();
            vita2d_swap_buffers();
            video->Unlock();
        }

        LogDebug("_DrawThread exit");
        sceKernelExitThread(0);
        return 0;
    }
}