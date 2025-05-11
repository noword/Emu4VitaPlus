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
#include "language_arch.h"

bool gRunning = true;
const char *gCoreName = nullptr;

void IntroMovingStatus::Reset()
{
    pos = VITA_WIDTH;
    delta = -1;
}

bool IntroMovingStatus::Update(const char *text)
{
    if (delay.TimeUp())
    {
        pos += delta;
        if (pos > VITA_WIDTH)
        {
            pos = VITA_WIDTH;
        }
        else if (-pos >= ImGui::CalcTextSize(text).x)
        {
            pos = VITA_WIDTH;
        }
    }

    return true;
}

App::App()
    : _index_x(0),
      _index_y(0),
      _start_count(60 * 6),
      _in_choice(false),
      _current_buttons(&_visable_buttons)
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
    My_ImGui_ImplVita2D_Init(gConfig->language, CACHE_DIR);
    ImGui_ImplVita2D_TouchUsage(false);
    ImGui_ImplVita2D_UseIndirectFrontTouch(false);
    ImGui_ImplVita2D_UseRearTouch(false);
    ImGui_ImplVita2D_GamepadUsage(false);

    ImGuiStyle *style = &ImGui::GetStyle();
    style->Colors[ImGuiCol_TitleBg] = style->Colors[ImGuiCol_TitleBgActive];

    _buttons = {
        new CoreButton(ATARI2600, // 1977
                       {{"Stella 2014", "stella2014"}}),
        new CoreButton(ATARI5200, // 1982
                       {{"Atari800", "atari800"}}),
        new CoreButton(ATARI7800, // 1986
                       {{"ProSystem", "prosystem"}}),
        new CoreButton(C64, // 1982.1
                       {{"the Versatile Commodore Emulator", "vice"}}),
        new CoreButton(VECTREX, // 1982.4
                       {{"vecx", "vecx"}}),
        new CoreButton(ZXS, // 1982.11
                       {{"fuse", "fuse"}}),
        new CoreButton(DOS, // 1981
                       {{"DOS BOX Pure", "dosbox_pure"}}),
        new CoreButton(PC98, // 1982
                       {{"Neko Project II", "nekop2"},
                        {"Neko Project II kai", "np2kai"}}),
        new CoreButton(MSX, // 1983
                       {{"blueMSX", "bluemsx"},
                        {"Marat Fayzullin's fMSX", "fmsx"}}),
        new CoreButton(NES, // 1983
                       {{"FCEUmm " ICON_STAR, "fceumm"},
                        {"Nestopia", "nestopia"}}),
        new CoreButton(AMIGA, // 1985
                       {{"uae4arm", "uae4arm"}}),
        new CoreButton(X68000, // 1987
                       {{"Portable (x)keropi PRO-68K", "px68k"}}),
        new CoreButton(PCE, // 1987
                       {{"Mednafen PCE Fast", "mednafen_pce_fast"},
                        {"Mednafen SuperGrafx", "mednafen_supergrafx"}}),
        new CoreButton(MD, // 1988
                       {{"Genesis Plus GX " ICON_STAR, "genesis_plus_gx"},
                        {"Genesis Plus GX Wide", "genesis_plus_gx_wide"},
                        {"PicoDrive", "picodrive"}}),
        new CoreButton(GBC, // 1989,1998
                       {{"Gambatte", "gambatte"},
                        {"TGB Dual", "tgbdual"}}),
        new CoreButton(SNES, // 1990
                       {{"Snes9x 2002", "snes9x2002"},
                        {"Snes9x 2005" ICON_STAR, "snes9x2005_plus"},
                        {"Snes9x 2010", "snes9x2010"},
                        {"Mednafen Supafaust", "mednafen_supafaust"},
                        {"Chimera SNES" ICON_STAR, "chimerasnes"}}),
        new CoreButton(NEOCD, // 1994
                       {{"neocd", "neocd"}}),
        new CoreButton(PS1, // 1994.12
                       {{"PCSX ReARMed", "pcsx_rearmed"}}),
        new CoreButton(NGP, // 1998
                       {{"Mednafen NeoPop", "mednafen_ngp"}}),
        new CoreButton(WSC, // 1999
                       {{"Mednafen Wswan", "mednafen_wswan"}}),
        new CoreButton(GBA, // 2001
                       {{"gpSP " ICON_STAR, "gpsp"},
                        {"VBA Next", "vba_next"}}),
        new CoreButton(ARC,
                       {{"FBA Lite", "fba_lite"},
                        {"FBA 2012", "fbalpha2012"},
                        {"FinalBurn Neo", "fbneo"},
                        {"FinalBurn Neo Xtreme", "km_fbneo_xtreme_amped"},
                        {"MAME 2003" ICON_STAR, "mame2003"},
                        {"MAME 2003 Plus", "mame2003_plus"},
                        {"MAME 2003 Xtreme" ICON_STAR, "km_mame2003_xtreme_amped"}}),
    };

    _input.SetTurboInterval(DEFAULT_TURBO_START_TIME * 5);
    SetInputHooks(&_input);

    sceKernelCreateLwMutex(&_video_mutex, "video_mutex", 0, 0, NULL);

    _SetVisableButtons();
    _RestoreLastCore();
    _UpdateIntro();
}

App::~App()
{
    LogFunctionName;

    delete gConfig;
    sceKernelDeleteLwMutex(&_video_mutex);

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

    if (ImGui::Begin("Emu4Vita++ v" APP_VER_STR, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs))
    {
        My_ImGui_ShowTimePower();

        ImVec2 pos = ImGui::GetWindowPos();

        if (_start_count > 0)
        {
            _start_count--;
            const char *manage_icons = gArchs[gConfig->language][LANG_MANAGE_ICONS];
            ImVec2 size = ImGui::CalcTextSize(manage_icons);
            ImGui::GetWindowDrawList()->AddText({(VITA_WIDTH - size.x) / 2.f, 70.f}, IM_COL32_WHITE, manage_icons);
        }

        pos.y = (ImGui::GetContentRegionMax().y - BUTTON_SIZE * 2) / 2 + 20;
        ImGui::SetCursorPos(pos);
        size_t count = 0;
        size_t index = _GetIndex();

        _VideoLock();
        for (auto button : *_current_buttons)
        {
            bool selected = (count == index);
            button->Show(selected, _in_choice);
            if (selected)
            {
                size_t half = (_current_buttons->size() + 1) / 2;
                ImGui::SetScrollHereX(float(count % half) / float(half));
            }

            count++;

            if (count != (_current_buttons->size() + 1) / 2)
            {
                ImGui::SameLine();
            }
            else
            {
                pos.y += BUTTON_SIZE + 8;
                ImGui::SetCursorPos(pos);
            }
        }
        _VideoUnlock();

        if (_current_buttons->size() & 1)
        {
            // show an empty button
            ImGui::Button("", {BUTTON_SIZE, BUTTON_SIZE});
        }

        if (*_intro)
        {
            ImVec2 size = ImGui::CalcTextSize(_intro);
            ImGui::SetNextWindowPos({MAIN_WINDOW_PADDING, VITA_HEIGHT - MAIN_WINDOW_PADDING * 2 - size.y});
            ImGui::SetNextWindowSize({VITA_WIDTH - MAIN_WINDOW_PADDING * 2, size.y});
            if (ImGui::Begin("info", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs))
            {
                ImGui::SetCursorPos({(float)_moving_status.pos, 0.f});
                ImGui::TextUnformatted(_intro);
                _moving_status.Update(_intro);

                if (_start_count > 0)
                {
                    bool push_color = (_start_count / 60) % 2 == 0;
                    if (push_color)
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_DARK_GREY);

                    ImGui::SetCursorPos({_moving_status.pos - ImGui::CalcTextSize(BUTTON_RIGHT_ANALOG_LEFT_RIGHT).x * 1.5, 0});
                    ImGui::TextUnformatted(BUTTON_RIGHT_ANALOG_LEFT_RIGHT);

                    if (push_color)
                        ImGui::PopStyleColor();
                }
            }
            ImGui::End();
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
    input->SetKeyDownCallback(SCE_CTRL_START, std::bind(&App::_OnKeyStart, this, input));
    input->SetKeyUpCallback(EnterButton, std::bind(&App::_OnClick, this, input));
    input->SetKeyDownCallback(SCE_CTRL_RSTICK_LEFT, std::bind(&App::_OnStartRollingIntro, this, input));
    input->SetKeyUpCallback(SCE_CTRL_RSTICK_LEFT, std::bind(&App::_OnStopRollingIntro, this, input));
    input->SetKeyDownCallback(SCE_CTRL_RSTICK_RIGHT, std::bind(&App::_OnStartRollingBackIntro, this, input));
    input->SetKeyUpCallback(SCE_CTRL_RSTICK_RIGHT, std::bind(&App::_OnStopRollingIntro, this, input));
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
    input->UnsetKeyDownCallback(SCE_CTRL_START);
    input->UnsetKeyUpCallback(EnterButton);
    input->UnsetKeyDownCallback(SCE_CTRL_RSTICK_LEFT);
    input->UnsetKeyUpCallback(SCE_CTRL_RSTICK_LEFT);
    input->UnsetKeyDownCallback(SCE_CTRL_RSTICK_RIGHT);
    input->UnsetKeyUpCallback(SCE_CTRL_RSTICK_RIGHT);
}

void App::_OnKeyLeft(Input *input)
{
    LOOP_MINUS_ONE(_index_x, (_current_buttons->size() + 1) / ROW_COUNT);
    _UpdateIntro();
}

void App::_OnKeyRight(Input *input)
{
    LOOP_PLUS_ONE(_index_x, (_current_buttons->size() + 1) / ROW_COUNT);
    _UpdateIntro();
}

void App::_OnKeyUp(Input *input)
{
    LOOP_MINUS_ONE(_index_y, ROW_COUNT);
    _UpdateIntro();
}

void App::_OnKeyDown(Input *input)
{
    LOOP_PLUS_ONE(_index_y, ROW_COUNT);
    _UpdateIntro();
}

void App::_OnClick(Input *input)
{
    LogFunctionName;
    if (_in_choice)
    {
        size_t count = 0;
        for (size_t i = 0; i < CONSOLE_COUNT && count <= 1; i++)
        {
            if (gConfig->consoles[i])
            {
                count += 1;
            }
        }

        if (count > 1)
            gConfig->consoles[_GetIndex()] = !gConfig->consoles[_GetIndex()];
    }
    else
    {
        _visable_buttons[_GetIndex()]->OnActive(input);
    }
}

void App::_OnKeyStart(Input *input)
{
    LogFunctionName;
    _in_choice = !_in_choice;
    if (_in_choice)
    {
        _current_buttons = &_buttons;
    }
    else
    {
        _SetVisableButtons();
        _current_buttons = &_visable_buttons;
        _UpdateIntro();
        gConfig->Save();
    }
}

size_t App::_GetIndex()
{
    size_t index = _index_y * ((_current_buttons->size() + 1) / ROW_COUNT) + _index_x;
    if (index >= _current_buttons->size())
    {
        index = _current_buttons->size() - 1;
    }
    return index;
}

void App::_UpdateIntro()
{
    LogFunctionName;
    _moving_status.Reset();
    _intro = (*_current_buttons)[_GetIndex()]->GetIntro();
}

void App::_OnStartRollingIntro(Input *input)
{
    _moving_status.delta = -5;
}

void App::_OnStopRollingIntro(Input *input)
{
    _moving_status.delta = -1;
}

void App::_OnStartRollingBackIntro(Input *input)
{
    _moving_status.delta = 5;
}

void App::_SetVisableButtons()
{
    LogFunctionName;
    _VideoLock();

    _visable_buttons.clear();
    for (size_t i = 0; i < CONSOLE_COUNT; i++)
    {
        LogDebug("%s %d", CONSOLE_NAMES[i], gConfig->consoles[i]);
        if (gConfig->consoles[i])
        {
            _visable_buttons.push_back(_buttons[i]);
        }
    }

    _VideoUnlock();
}

int32_t App::_VideoLock(uint32_t *timeout)
{
    return sceKernelLockLwMutex(&_video_mutex, 1, timeout);
}

void App::_VideoUnlock()
{
    sceKernelUnlockLwMutex(&_video_mutex, 1);
}

void App::_RestoreLastCore()
{
    bool found = false;
    for (size_t i = 0; i < _visable_buttons.size() && !found; i++)
    {
        CoreButton *button = _visable_buttons[i];
        for (size_t j = 0; j < button->_cores.size(); j++)
        {
            if (gConfig->last_core == button->_cores[j].boot_name)
            {
                _index_x = i % (_visable_buttons.size() / ROW_COUNT);
                _index_y = i / (_visable_buttons.size() / ROW_COUNT);
                button->_index = j;
                found = true;
                break;
            }
        }
    }
}
