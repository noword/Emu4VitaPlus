#pragma once
#include <vector>
#include "my_imgui.h"
#include "input.h"
#include "core_button.h"
#include "file.h"

extern bool gRunning;
extern char gCorePath[SCE_FIOS_PATH_MAX];

struct IntroMovingStatus : public TextMovingStatus
{
    void Reset();
    bool Update(const char *text);
};

class App
{
public:
    App();
    virtual ~App();
    void Run();
    void SetInputHooks(Input *input);
    void UnsetInputHooks(Input *input);

private:
    void _Show();
    size_t _GetIndex();
    void _UpdateIntro();
    void _SetVisableButtons();

    void _OnClick(Input *input);
    void _OnKeyLeft(Input *input);
    void _OnKeyRight(Input *input);
    void _OnKeyUp(Input *input);
    void _OnKeyDown(Input *input);
    void _OnKeyStart(Input *input);
    void _OnStartRollingIntro(Input *input);
    void _OnStopRollingIntro(Input *input);
    void _OnStartRollingBackIntro(Input *input);
    int32_t _VideoLock(uint32_t *timeout = NULL);
    void _VideoUnlock();

    Input _input;
    std::vector<CoreButton *> _buttons;
    std::vector<CoreButton *> _visable_buttons;
    std::vector<CoreButton *> *_current_buttons;

    size_t _index_x;
    size_t _index_y;
    IntroMovingStatus _moving_status;
    const char *_intro;

    int _show_strick_count;

    SceKernelLwMutexWork _video_mutex;
    bool _in_choice;
};