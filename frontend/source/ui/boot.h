#pragma once
#include <vector>
#include <string>
#include "input.h"
#include "my_imgui.h"

using namespace Emu4VitaPlus;

class Boot
{
public:
    Boot();
    virtual ~Boot();

    void Show();
    void Run();
    void AppendLog(const char *log);
    void ClearLogs();
    void SetInputHooks();
    void UnsetInputHooks();

private:
    void _OnKey(Input *input);
    void _OnKeyUp(Input *input);
    void _OnKeyDown(Input *input);

    Input _input;
    std::vector<std::string> _logs;
    My_Imgui_SpinText _spin_text;
    size_t _index;
};