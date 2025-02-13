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
    void AppendLog(const char *log);
    void ClearLogs();
    void SetInputHooks(Input *input);
    void UnsetInputHooks(Input *input);

private:
    void _OnKey(Input *input);
    void _OnKeyUp(Input *input);
    void _OnKeyDown(Input *input);

    std::vector<std::string> _logs;
    My_Imgui_SpinText _spin_text;
    bool _show_spin;
    size_t _index;
};