#include "touch.h"
#include "log.h"

#define MAX_STATES 256

SceTouchPanelInfo Touch::_info[2] = {0};

Touch::Touch(SceTouchPortType port)
    : _port(port),
      _enabled(false),
      _x_scale(1.f),
      _y_scale(1.f),
      _down_count(0)
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
}

Touch::~Touch()
{
    LogFunctionName;
}

void Touch::Enable(bool enable)
{
    LogFunctionName;
    _enabled = enable;
    sceTouchSetSamplingState(_port, enable ? SCE_TOUCH_SAMPLING_STATE_START : SCE_TOUCH_SAMPLING_STATE_STOP);
}

void Touch::Poll()
{
    if (!_enabled)
    {
        return;
    }

    SceTouchData touch_data{0};
    if (sceTouchPeek(_port, &touch_data, 1) == 1)
    {
        _current_id = touch_data.report->id;
        _axis.x = touch_data.report->x >> 1;
        _axis.y = touch_data.report->y >> 1;

        _locker.Lock();

        if (_current_id == _last_id)
        {
            if (_axis == _last_axis)
            {
                if (_down_count < 10)
                {
                    _down_count++;
                    _states.push(TouchHold);
                }
                else if (_down_count == 10)
                {
                    _down_count++;
                    _states.push(TouchUp);
                }
            }
            else
            {
                _states.push(TouchHold);
            }
        }
        else
        {
            _states.push(TouchDown);
            _down_count = 0;
        }

        if (_states.size() > MAX_STATES)
        {
            _states.pop();
        }

        _locker.Unlock();

        _last_id = _current_id;
        _last_axis = _axis;
    }
}

TouchState Touch::GetState()
{
    if (_states.empty())
    {
        return TouchNone;
    }

    _locker.Lock();
    TouchState state = _states.front();
    _states.pop();
    _locker.Unlock();

    return state;
}

void Touch::InitMovingScale(float xscale, float yscale)
{
    LogFunctionName;
    size_t sizex = _info[_port].maxAaX - _info[_port].minAaX;
    size_t sizey = _info[_port].maxAaY - _info[_port].minAaY;
    _locker.Lock();

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

    _locker.Unlock();
}