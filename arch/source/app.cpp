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
#include "theme.h"

#define MIN_COVER_ALPHA 60
#define MAX_COVER_ALPHA 230

bool gRunning = true;
std::string gCoreName;

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
      _current_buttons(&_visable_buttons),
      _max_row(0)
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

    Themes themes(DEFAULT_THEMES_JSON);
    themes.Apply(gConfig->theme.c_str());

    _buttons = {
        {
            new CoreButton(C64, // 1982.1
                           {{"the Versatile Commodore Emulator", "vice"}}),
            new CoreButton(ZXS, // 1982.11
                           {{"fuse", "fuse"}}),
            new CoreButton(DOS, // 1981
                           {{"DOS BOX Pure", "dosbox_pure"}}),
            new CoreButton(PC88, // 1981
                           {{"QUASI88", "quasi88"}}),
            new CoreButton(PC98, // 1982
                           {{"Neko Project II", "nekop2"},
                            {"Neko Project II kai", "np2kai"}}),
            new CoreButton(MSX, // 1983
                           {{"blueMSX", "bluemsx"},
                            {"Marat Fayzullin's fMSX", "fmsx"}}),
            new CoreButton(CPC, // 1984
                           {{"Caprice32", "cap32"},
                            {"CrocoDS", "crocods"}}),
            new CoreButton(AMIGA, // 1985
                           {{"uae4arm", "uae4arm"}}),
            new CoreButton(X68000, // 1987
                           {{"Portable (x)keropi PRO-68K", "px68k"}}),

        },
        {
            new CoreButton(ATARI2600, // 1977
                           {{"Stella 2014", "stella2014"}}),
            new CoreButton(ATARI5200, // 1982
                           {{"Atari800", "atari800"}}),
            new CoreButton(ATARI7800, // 1986
                           {{"ProSystem", "prosystem"}}),

            new CoreButton(VECTREX, // 1982.4
                           {{"vecx", "vecx"}}),

            new CoreButton(NES, // 1983
                           {{"FCEUmm " ICON_STAR, "fceumm"},
                            {"Nestopia", "nestopia"}}),

            new CoreButton(PCE, // 1987
                           {{"Mednafen PCE Fast", "mednafen_pce_fast"},
                            {"Mednafen SuperGrafx", "mednafen_supergrafx"}}),
            new CoreButton(MD, // 1988
                           {{"Genesis Plus GX " ICON_STAR, "genesis_plus_gx"},
                            {"Genesis Plus GX Wide", "genesis_plus_gx_wide"},
                            {"PicoDrive", "picodrive"}}),
            new CoreButton(SNES, // 1990
                           {     //{"Snes9x 2002", "snes9x2002"},
                            {"Snes9x 2005" ICON_STAR, "snes9x2005_plus"},
                            {"Snes9x", "snes9x"},
                            {"Mednafen Supafaust" ICON_STAR, "mednafen_supafaust"},
                            {"Chimera SNES", "chimerasnes"}}),
            new CoreButton(NEOCD, // 1994
                           {{"neocd", "neocd"}}),
            new CoreButton(VB, // 1995
                           {{"Beetle VB", "mednafen_vb"}}),
            new CoreButton(PS1, // 1994.12
                           {{"PCSX ReARMed", "pcsx_rearmed"}}),
        },
        {
            new CoreButton(GW, // 1980
                           {{"Handheld Electronic (GW)", "gw"}}),
            new CoreButton(LYNX, // 1989
                           {{"Beetle Lynx", "mednafen_lynx"},
                            {"Handy", "handy"}}),
            new CoreButton(GBC, // 1989,1998
                           {{"Gambatte", "gambatte"},
                            {"TGB Dual", "tgbdual"}}),

            new CoreButton(NGP, // 1998
                           {{"Mednafen NeoPop", "mednafen_ngp"}}),
            new CoreButton(WSC, // 1999
                           {{"Mednafen Wswan", "mednafen_wswan"}}),
            new CoreButton(GBA, // 2001
                           {{"gpSP " ICON_STAR, "gpsp"},
                            {"mGBA", "mgba"},
                            {"VBA Next", "vba_next"}}),
        },
        {
            new CoreButton(ARC,
                           {{"FBA Lite", "fba_lite"},
                            {"FBA 2012", "fbalpha2012"},
                            {"FinalBurn Neo", "fbneo"},
                            {"FinalBurn Neo Xtreme", "km_fbneo_xtreme_amped"},
                            {"MAME 2000", "mame2000"},
                            {"MAME 2003" ICON_STAR, "mame2003"},
                            {"MAME 2003 Plus", "mame2003_plus"},
                            {"MAME 2003 Xtreme" ICON_STAR, "km_mame2003_xtreme_amped"}}),
        }};

    _input.SetTurboInterval(DEFAULT_TURBO_START_TIME * 5);
    SetInputHooks(&_input);

    sceKernelCreateLwMutex(&_video_mutex, "video_mutex", 0, 0, NULL);

    _SetVisableButtons();
    _SetMaxRow();
    _RestoreLastCore();
    _UpdateIntro();
}

void App::_SetMaxRow()
{
    for (const auto &buttons : *_current_buttons)
    {
        _max_row = std::max(_max_row, (float)buttons.size());
    }
}

App::~App()
{
    LogFunctionName;

    delete gConfig;
    sceKernelDeleteLwMutex(&_video_mutex);

    for (auto buttons : _buttons)
    {
        for (auto button : buttons)
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

    if (ImGui::Begin("Emu4Vita++ v" APP_VER_STR, NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs))
    {
        My_ImGui_ShowTimePower();

        if (_start_count > 0)
        {
            _start_count--;
            const char *manage_icons = gArchs[gConfig->language][LANG_MANAGE_ICONS];
            ImGui::GetWindowDrawList()->AddText({(VITA_WIDTH - ImGui::CalcTextSize(manage_icons).x) / 2.f, 70.f},
                                                IM_COL32_WHITE,
                                                manage_icons);
        }

        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        ImVec2 main_size{avail_size.x - COVER_WIDTH - 8, avail_size.y};
        ImVec2 intro_size;
        if (*_intro)
        {
            ImVec2 intro_size = ImGui::CalcTextSize(_intro);
            main_size.y -= intro_size.y;
        }
        vita2d_texture *cover = nullptr;

        _VideoLock();

        if (ImGui::BeginChild("arch", main_size, false, ImGuiWindowFlags_NoScrollbar))
        {
            ImVec2 pos = ImGui::GetCursorPos();

            for (size_t y = 0; y < _current_buttons->size(); y++)
            {
                const CoreButtons &buttons = (*_current_buttons)[y];
                for (size_t x = 0; x < buttons.size(); x++)
                {
                    bool selected = _index_y == y && _index_x == x;
                    buttons[x]->Show(selected, _in_choice);
                    if (selected)
                    {
                        ImGui::SetScrollHereX((float)x / (float)buttons.size());
                        ImGui::SetScrollHereY((float)y / (float)_current_buttons->size());
                        cover = buttons[x]->GetCover();
                    }

                    ImGui::SameLine();
                }

                pos.y += BUTTON_SIZE + 8;
                ImGui::SetCursorPos(pos);
            }
        }
        ImGui::EndChild();

        _VideoUnlock();

        ImGui::SameLine();
        if (cover)
        {
            if (_cover_alpha < MAX_COVER_ALPHA)
                _cover_alpha++;

            if (ImGui::BeginChild("cover"))
            {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(pos,
                                                          ImVec2(pos.x + COVER_WIDTH, pos.y + COVER_HEIGHT),
                                                          IM_COL32(255, 255, 255, _cover_alpha));
                ImGui::Image(cover, {COVER_WIDTH, COVER_HEIGHT});
            }
            ImGui::EndChild();
        }

        if (*_intro)
        {
            ImGui::SetNextWindowPos({MAIN_WINDOW_PADDING, VITA_HEIGHT - MAIN_WINDOW_PADDING * 2.5 - intro_size.y});
            ImGui::SetNextWindowSize({VITA_WIDTH - MAIN_WINDOW_PADDING * 2, intro_size.y});
            if (ImGui::Begin("intro", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs))
            {
                ImGui::SetCursorPos({(float)_moving_status.pos, 0});
                ImGui::TextUnformatted(_intro);
                _moving_status.Update(_intro);

                if (_start_count > 0)
                {
                    ImGui::SetCursorPos({_moving_status.pos - ImGui::CalcTextSize(BUTTON_RIGHT_ANALOG_LEFT_RIGHT).x * 1.5, 0});

                    if ((_start_count / 60) % 2 == 0)
                        ImGui::TextDisabled(BUTTON_RIGHT_ANALOG_LEFT_RIGHT);
                    else
                        ImGui::TextUnformatted(BUTTON_RIGHT_ANALOG_LEFT_RIGHT);
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
    if (_index_x > 0)
        _index_x--;
    _UpdateIntro();
}

void App::_OnKeyRight(Input *input)
{
    if (_index_x + 1 < (*_current_buttons)[_index_y].size())
        _index_x++;

    _UpdateIntro();
}

void App::_OnKeyUp(Input *input)
{
    if (_index_y > 0)
    {
        _index_y--;
        if (_index_x >= (*_current_buttons)[_index_y].size())
            _index_x = (*_current_buttons)[_index_y].size() - 1;
    }

    _UpdateIntro();
}

void App::_OnKeyDown(Input *input)
{
    if (_index_y + 1 < _current_buttons->size())
    {
        _index_y++;
        if (_index_x >= (*_current_buttons)[_index_y].size())
            _index_x = (*_current_buttons)[_index_y].size() - 1;
    }
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

        CONSOLE console = _buttons[_index_y][_index_x]->GetConsole();
        if (count > 1 || !gConfig->consoles[console]) // at least, keep one icon
        {
            gConfig->consoles[console] = !gConfig->consoles[console];
        }
    }
    else
    {
        _visable_buttons[_index_y][_index_x]->OnActive(input);
    }
}

void App::_OnKeyStart(Input *input)
{
    LogFunctionName;

    CONSOLE console = (*_current_buttons)[_index_y][_index_x]->GetConsole();
    _in_choice = !_in_choice;
    if (_in_choice)
    {
        _current_buttons = &_buttons;
        _SetIndex(console);
    }
    else
    {
        _SetVisableButtons();
        _current_buttons = &_visable_buttons;
        gConfig->Save();
        if (!_SetIndex(console))
        {
            if (_index_y >= _current_buttons->size())
                _index_y = _current_buttons->size() - 1;

            if (_index_x >= (*_current_buttons)[_index_y].size())
                _index_x = (*_current_buttons)[_index_y].size() - 1;
        }
        _UpdateIntro();
    }

    _SetMaxRow();
}

bool App::_SetIndex(CONSOLE console)
{
    size_t x = 0;
    size_t y = 0;
    bool found = false;

    for (const auto buttons : *_current_buttons)
    {
        for (const auto button : buttons)
        {
            if (console == button->GetConsole())
            {
                found = true;
                break;
            }
            x++;
        }

        if (found)
        {
            break;
        }
        else
        {
            x = 0;
            y++;
        }
    }

    if (found)
    {
        _index_x = x;
        _index_y = y;
    }

    return found;
}

void App::_UpdateIntro()
{
    LogFunctionName;
    _cover_alpha = MIN_COVER_ALPHA;
    _moving_status.Reset();
    _intro = (*_current_buttons)[_index_y][_index_x]->GetIntro();
    (*_current_buttons)[_index_y][_index_x]->ResetAlpha();
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
    for (auto buttons : _buttons)
    {
        CoreButtons core_buttons;
        for (auto button : buttons)
        {
            CONSOLE console = button->GetConsole();
            LogDebug("%s %d", CONSOLE_NAMES[console], gConfig->consoles[console]);
            if (gConfig->consoles[console])
            {
                core_buttons.push_back(button);
            }
        }
        if (core_buttons.size() > 0)
            _visable_buttons.push_back(core_buttons);
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
    LogFunctionName;

    bool found = false;
    _index_x = _index_y = 0;
    for (auto buttons : _visable_buttons)
    {
        _index_x = 0;
        for (auto button : buttons)
        {
            for (size_t j = 0; j < button->_cores.size(); j++)
            {
                if (gConfig->last_core == button->_cores[j].boot_name)
                {
                    button->_index = j;
                    found = true;
                    break;
                }
            }

            if (found)
                break;

            _index_x++;
        }

        if (found)
            break;

        _index_y++;
    }

    if (!found)
    {
        _index_x = _index_y = 0;
    }

    LogDebug("%d %d %s", _index_x, _index_y, gConfig->last_core.c_str());
}
