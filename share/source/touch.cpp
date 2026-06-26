#include <string.h>
#include "touch.h"
#include "log.h"

#define MAX_STATES 256

namespace Emu4VitaPlus
{
    SceTouchPanelInfo Touch::_info[2] = {0};

    Touch::Touch(SceTouchPortType port)
        : _port(port),
          _enabled(false),
          _x_scale(1.f),
          _y_scale(1.f),
          _touched(false)
    {
        LogFunctionName;

        if (port != 0 && port != 1)
        {
            LogError("Wrong Port %d, force to front", port);
            _port = SCE_TOUCH_PORT_FRONT;
        }

        if (_info[_port].maxAaX == 0)
        {
            sceTouchGetPanelInfo(_port, &_info[_port]);
            LogDebug("%d:\n"
                     "   %d %d %d %d\n"
                     "   %d %d %d %d",
                     _port,
                     _info[_port].minAaX, _info[_port].minAaY, _info[_port].maxAaX, _info[_port].maxAaY,
                     _info[_port].minDispX, _info[_port].minDispY, _info[_port].maxDispX, _info[_port].maxDispY);
        }

        _center.x = (_info[_port].maxAaX - _info[_port].minAaX) / 4;
        _center.y = (_info[_port].maxAaY - _info[_port].minAaY) / 4;

        memset(_states, 0, sizeof(_states));
    }

    Touch::~Touch()
    {
        LogFunctionName;
    }

    void Touch::Enable(bool enable)
    {
        LogFunctionName;
        LogDebug("  enable: %d", enable);
        _enabled = enable;
        sceTouchSetSamplingState(_port, enable ? SCE_TOUCH_SAMPLING_STATE_START : SCE_TOUCH_SAMPLING_STATE_STOP);
    }

    void Touch::Poll()
    {
        if (!_enabled)
        {
            return;
        }

        int current = _current.load(std::memory_order_relaxed) ^ 1;
        TouchState *state = _states + current;
        state->Reset();

        SceTouchData touch_data{0};
        _touched = (sceTouchPeek(_port, &touch_data, 1) >= SCE_OK && touch_data.reportNum > 0);
        if (_touched)
        {
            for (auto i = 0; i < touch_data.reportNum; i++)
                state->Set(&touch_data.report[i]);
        }

        _current.store(current, std::memory_order_release);
    }

    void Touch::InitMovingScale(float xscale, float yscale)
    {
        LogFunctionName;
        size_t sizex = _info[_port].maxAaX - _info[_port].minAaX;
        size_t sizey = _info[_port].maxAaY - _info[_port].minAaY;

        LogDebug("  _port: %d sizex: %d sizey: %d", _port, sizex, sizey);

        _scale_map_table_x.clear();
        _scale_map_table_x.reserve(sizex);
        for (size_t i = 0; i < sizex; i++)
        {
            _scale_map_table_x.emplace_back(i * xscale);
        }

        _scale_map_table_y.clear();
        _scale_map_table_y.reserve(sizey);
        for (size_t i = 0; i < sizey; i++)
        {
            _scale_map_table_y.emplace_back(i * xscale);
        }
    }
}