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
    bool speed_up = false;
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

    void _OnClick(Input *input);
    void _OnKeyLeft(Input *input);
    void _OnKeyRight(Input *input);
    void _OnKeyUp(Input *input);
    void _OnKeyDown(Input *input);
    void _OnStartSpeedUpIntro(Input *input);
    void _OnStopSpeedUpIntro(Input *input);

    Input _input;
    std::vector<CoreButton *> _buttons;
    size_t _index_x;
    size_t _index_y;
    IntroMovingStatus _moving_status;
    const char *_intro;
};