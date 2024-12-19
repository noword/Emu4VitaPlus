#pragma once
#include <vita2d.h>
#include <imgui_vita2d/imgui_vita.h>
#include <string>
#include <vector>
#include "input.h"

using namespace Emu4VitaPlus;

struct CoreName
{
    std::string name;
    std::string boot_name;
};

class App;

class CoreButton
{
    friend class App;

public:
    CoreButton(std::string name, std::vector<CoreName> cores);
    virtual ~CoreButton();
    void Show(bool selected);
    void OnActive(Input *input);
    void SetInputHooks(Input *input);
    void UnsetInputHooks(Input *input);

private:
    void _ShowPopup();
    void _OnKeyUp(Input *input);
    void _OnKeyDown(Input *input);
    void _OnClick(Input *input);
    void _OnCancel(Input *input);
    void _BootCore();

    std::string _name;
    std::vector<CoreName> _cores;
    vita2d_texture *_texture;
    bool _actived;
    size_t _index;
};