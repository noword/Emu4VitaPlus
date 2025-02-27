#include <psp2/ctrl.h>
#include <psp2/power.h>
#include <psp2/touch.h>
#include <psp2/apputil.h>
#include <psp2/display.h>
#include <vita2d.h>
#include "my_imgui.h"
#include "log.h"
#include "app.h"
#include "defines.h"
#include "utils.h"
#include "config.h"
#include "icons.h"

bool gRunning = true;
char gCorePath[SCE_FIOS_PATH_MAX] = {0};

App::App() : _index_x(0), _index_y(0)
{
    LogFunctionName;

    // scePowerSetArmClockFrequency(444);
    // scePowerSetBusClockFrequency(222);
    // scePowerSetGpuClockFrequency(222);
    // scePowerSetGpuXbarClockFrequency(166);

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
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

    EnterButton = config.enterButtonAssign == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE ? SCE_CTRL_CIRCLE : SCE_CTRL_CROSS;

    vita2d_init();
    vita2d_set_vblank_wait(1);

    ImGui::CreateContext();
    My_ImGui_ImplVita2D_Init(0);
    ImGui_ImplVita2D_TouchUsage(false);
    ImGui_ImplVita2D_UseIndirectFrontTouch(false);
    ImGui_ImplVita2D_UseRearTouch(false);
    ImGui_ImplVita2D_GamepadUsage(false);

    ImGuiStyle *style = &ImGui::GetStyle();
    style->Colors[ImGuiCol_TitleBg] = style->Colors[ImGuiCol_TitleBgActive];

    _buttons = {
        new CoreButton("ATARI2600", {{"Stella 2014", "Stella2014"}}),      // 1977
        new CoreButton("ATARI5200", {{"Atari800", "Atari800"}}),           // 1982
        new CoreButton("ATARI7800", {{"ProSystem", "ProSystem"}}),         // 1986
        new CoreButton("VECTREX", {{"vecx", "vecx"}}),                     // 1982.4
        new CoreButton("ZXS", {{"fuse", "fuse"}}),                         // 1982.11
        new CoreButton("DOS", {{"DOS BOX Pure" ICON_STAR, "DOSBoxPure"}}), // 1981
        new CoreButton("PC98", {{"Neko Project II", "nekop2"}}),           // 1982
        new CoreButton("MSX", {{"blueMSX", "blueMSX"},                     // 1983
                               {"Marat Fayzullin's fMSX", "fMSX"}}),
        new CoreButton("NES", {{"FCEUmm " ICON_STAR, "FCEUmm"}, // 1983
                               {"Nestopia", "Nestopia"}}),
        new CoreButton("AMIGA", {{"uae4arm", "uae4arm"}}),               // 1985
        new CoreButton("PCE", {{"Mednafen PCE Fast", "MednafenPCEFast"}, // 1987
                               {"Mednafen SuperGrafx", "MednafenPCESuperGrafx"}}),
        new CoreButton("MD", {{"Genesis Plus GX " ICON_STAR, "GenesisPlusGX"}, // 1988
                              {"Genesis Plus GX Wide", "GenesisPlusGXWide"},
                              {"PicoDrive", "PicoDrive"}}),
        new CoreButton("GBC", {{"Gambatte", "Gambatte"}, // 1989,1998
                               {"TGB Dual", "TGBDual"},
                               {"mGBA", "mGBA"}}),
        new CoreButton("SNES", {{"Snes9x 2002", "Snes9x2002"}, // 1990
                                {"Snes9x 2005 ", "Snes9x2005"},
                                {"Snes9x 2010", "Snes9x2010"},
                                {"Mednafen Supafaust", "Supafaust"},
                                {"Chimera SNES" ICON_STAR, "ChimeraSNES"}}),
        new CoreButton("NEOCD", {{"neocd", "neocd"}}),                // 1994
        new CoreButton("PS1", {{"PCSX ReARMed", "PCSXReARMed"}}),     // 1994.12
        new CoreButton("NGP", {{"Mednafen NeoPop", "MednafenNgp"}}),  // 1998
        new CoreButton("WSC", {{"Mednafen Wswan", "MednafenWswan"}}), // 1999
        new CoreButton("GBA", {{"gpSP " ICON_STAR, "gpSP"},           // 2001
                               {"VBA Next", "VBANext"},
                               {"mGBA", "mGBA"}}),
        new CoreButton("ARC", {{"FBA Lite", "FBALite"},
                               {"FBA 2012", "FBA2012"},
                               {"FinalBurn Neo", "FBNeo"},
                               {"FinalBurn Neo Xtreme", "FBNeoXtreme"},
                               {"MAME 2000", "MAME2000"},
                               {"MAME 2003" ICON_STAR, "MAME2003"},
                               {"MAME 2003 Plus", "MAME2003Plus"},
                               {"MAME 2003 Xtreme" ICON_STAR, "MAME2003XTREME"}}),

        // new CoreButton("ATARIST", {{"Hatari", "hatari"}}),
    };

    SetInputHooks(&_input);

    bool found = false;
    for (size_t i = 0; i < _buttons.size() && !found; i++)
    {
        CoreButton *button = _buttons[i];
        for (size_t j = 0; j < button->_cores.size(); j++)
        {
            if (gConfig->last_core == button->_cores[j].boot_name)
            {
                _index_x = i % (_buttons.size() / ROW_COUNT);
                _index_y = i / (_buttons.size() / ROW_COUNT);
                button->_index = j;
                found = true;
                break;
            }
        }
    }
}

App::~App()
{
    LogFunctionName;

    for (auto button : _buttons)
    {
        delete button;
    }

    My_ImGui_ImplVita2D_Shutdown();
    ImGui::DestroyContext();

    // vita2d_fini();
}

void App::Run()
{
    while (gRunning)
    {
        _input.Poll();
        _Show();
    }
}

void App::_Show()
{
    LogFunctionNameLimited;
    // vita2d_set_clip_rectangle(0, 0, VITA_WIDTH, VITA_HEIGHT);
    vita2d_pool_reset();
    vita2d_start_drawing_advanced(NULL, 0);
    vita2d_clear_screen();

    ImGui_ImplVita2D_NewFrame();
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    ImGui::SetNextWindowPos({MAIN_WINDOW_PADDING, MAIN_WINDOW_PADDING});
    ImGui::SetNextWindowSize({VITA_WIDTH - MAIN_WINDOW_PADDING * 2, VITA_HEIGHT - MAIN_WINDOW_PADDING * 2});

    ImGui::Begin("Emu4Vita++ v" APP_VER_STR, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoInputs);
    My_Imgui_ShowTimePower();
    ImVec2 pos = ImGui::GetWindowPos();
    ImGui::SetCursorPos({pos.x, (ImGui::GetContentRegionMax().y - BUTTON_SIZE * 2) / 2 + 20});
    size_t count = 0;
    size_t index = _GetIndex();
    for (auto button : _buttons)
    {
        bool selected = (count == index);
        button->Show(selected);
        if (selected)
        {
            size_t half = (_buttons.size() + 1) / 2;

            ImGui::SetScrollHereX(float(count % half) / float(half));
        }

        count++;

        if (count != _buttons.size() / 2)
        {
            ImGui::SameLine();
        }
    }

    ImGui::End();
    ImGui::Render();
    My_ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());

    vita2d_end_drawing();
    vita2d_swap_buffers();
    return;
}

void App::SetInputHooks(Input *input)
{
    input->SetKeyDownCallback(SCE_CTRL_LEFT, std::bind(&App::_OnKeyLeft, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_RIGHT, std::bind(&App::_OnKeyRight, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_UP, std::bind(&App::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_DOWN, std::bind(&App::_OnKeyDown, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_LEFT, std::bind(&App::_OnKeyLeft, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_RIGHT, std::bind(&App::_OnKeyRight, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_UP, std::bind(&App::_OnKeyUp, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_LSTICK_DOWN, std::bind(&App::_OnKeyDown, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_L1, std::bind(&App::_OnKeyLeft, this, input), true);
    input->SetKeyDownCallback(SCE_CTRL_R1, std::bind(&App::_OnKeyRight, this, input), true);
    input->SetKeyUpCallback(SCE_CTRL_CIRCLE, std::bind(&App::_OnClick, this, input));
}

void App::UnsetInputHooks(Input *input)
{
    input->UnsetKeyDownCallback(SCE_CTRL_LEFT);
    input->UnsetKeyDownCallback(SCE_CTRL_RIGHT);
    input->UnsetKeyDownCallback(SCE_CTRL_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_DOWN);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_LEFT);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_RIGHT);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_UP);
    input->UnsetKeyDownCallback(SCE_CTRL_LSTICK_DOWN);
    input->UnsetKeyDownCallback(SCE_CTRL_L1);
    input->UnsetKeyDownCallback(SCE_CTRL_R1);
    input->UnsetKeyUpCallback(SCE_CTRL_CIRCLE);
}

void App::_OnKeyLeft(Input *input)
{
    LOOP_MINUS_ONE(_index_x, (_buttons.size() + _index_y) / ROW_COUNT);
}

void App::_OnKeyRight(Input *input)
{
    LOOP_PLUS_ONE(_index_x, (_buttons.size() + _index_y) / ROW_COUNT);
}

void App::_OnKeyUp(Input *input)
{
    LOOP_MINUS_ONE(_index_y, ROW_COUNT);
}

void App::_OnKeyDown(Input *input)
{
    LOOP_PLUS_ONE(_index_y, ROW_COUNT);
}

void App::_OnClick(Input *input)
{
    LogFunctionName;
    _buttons[_GetIndex()]->OnActive(input);
}

size_t App::_GetIndex()
{
    return _index_y * _buttons.size() / ROW_COUNT + _index_x;
}