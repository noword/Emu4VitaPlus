#include <psp2/apputil.h>
#include <psp2/ctrl.h>
#include <psp2/system_param.h>
#include <string.h>
#include "input.h"
#include "log.h"

#define N_CTRL_PORTS 4
#define ANALOG_CENTER 128
#define ANALOG_THRESHOLD 64

uint32_t EnterButton = SCE_CTRL_CROSS;

const int16_t Emu4VitaPlus::Input::_analog_map_table[0x100] = {
    -32767, -32510, -32253, -31996, -31739, -31482, -31225, -30968, -30711, -30454, -30197, -29940, -29683, -29426, -29169, -28912,
    -28655, -28398, -28141, -27884, -27627, -27370, -27113, -26856, -26599, -26342, -26085, -25828, -25571, -25314, -25057, -24800,
    -24543, -24286, -24029, -23772, -23515, -23258, -23001, -22744, -22487, -22230, -21973, -21716, -21459, -21202, -20945, -20688,
    -20431, -20174, -19917, -19660, -19403, -19146, -18889, -18632, -18375, -18118, -17861, -17604, -17347, -17090, -16833, -16576,
    -16319, -16062, -15805, -15548, -15291, -15034, -14777, -14520, -14263, -14006, -13749, -13492, -13235, -12978, -12721, -12464,
    -12207, -11950, -11693, -11436, -11179, -10922, -10665, -10408, -10151, -9894, -9637, -9380, -9123, -8866, -8609, -8352,
    -8095, -7838, -7581, -7324, -7067, -6810, -6553, -6296, -6039, -5782, -5525, -5268, -5011, -4754, -4497, -4240,
    -3983, -3726, -3469, -3212, -2955, -2698, -2441, -2184, -1927, -1670, -1413, -1156, -899, -642, -385, -128,
    128, 385, 642, 899, 1156, 1413, 1670, 1927, 2184, 2441, 2698, 2955, 3212, 3469, 3726, 3983,
    4240, 4497, 4754, 5011, 5268, 5525, 5782, 6039, 6296, 6553, 6810, 7067, 7324, 7581, 7838, 8095,
    8352, 8609, 8866, 9123, 9380, 9637, 9894, 10151, 10408, 10665, 10922, 11179, 11436, 11693, 11950, 12207,
    12464, 12721, 12978, 13235, 13492, 13749, 14006, 14263, 14520, 14777, 15034, 15291, 15548, 15805, 16062, 16319,
    16576, 16833, 17090, 17347, 17604, 17861, 18118, 18375, 18632, 18889, 19146, 19403, 19660, 19917, 20174, 20431,
    20688, 20945, 21202, 21459, 21716, 21973, 22230, 22487, 22744, 23001, 23258, 23515, 23772, 24029, 24286, 24543,
    24800, 25057, 25314, 25571, 25828, 26085, 26342, 26599, 26856, 27113, 27370, 27627, 27884, 28141, 28398, 28655,
    28912, 29169, 29426, 29683, 29940, 30197, 30454, 30711, 30968, 31225, 31482, 31739, 31996, 32253, 32510, 32767};

void SwapEnterButton(bool swap)
{
    static uint32_t enter_button = 0;
    if (enter_button == 0)
    {
        int config;
        sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &config);
        enter_button = config == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE ? SCE_CTRL_CIRCLE : SCE_CTRL_CROSS;
    }

    if (swap)
    {
        EnterButton = enter_button == SCE_CTRL_CIRCLE ? SCE_CTRL_CROSS : SCE_CTRL_CIRCLE;
    }
    else
    {
        EnterButton = enter_button;
    }
}

namespace Emu4VitaPlus
{
    Input::Input() : _last_key(0ull),
                     _current_hotkey(0ull),
                     _turbo_key(0ull),
                     _turbo_start_ms(DEFAULT_TURBO_START_TIME),
                     _turbo_interval_ms(DEFAULT_TURBO_INTERVAL),
                     _enable_key_up(true),
                     _left_analog{0},
                     _right_analog{0}
    {
        LogFunctionName;
        memset(_turbo_key_states, 0, sizeof(_turbo_key_states));
    }

    Input::~Input()
    {
    }

    void Input::SetKeyDownCallback(uint32_t key, InputFunc func, bool turbo)
    {
        if (func == nullptr)
        {
            UnsetKeyDownCallback(key);
        }
        else
        {
            if (turbo)
            {
                _turbo_key |= key;
            }

            for (auto &iter : _key_down_callbacks)
            {
                if (iter.key == key)
                {
                    iter.func = func;
                    return;
                }
            }

            _key_down_callbacks.push_back({key, func});
        }
    }

    void Input::SetKeyUpCallback(uint32_t key, InputFunc func)
    {
        if (func == nullptr)
        {
            UnsetKeyUpCallback(key);
        }
        else
        {
            for (auto &iter : _key_up_callbacks)
            {
                if (iter.key == key)
                {
                    iter.func = func;
                    return;
                }
            }

            _key_up_callbacks.push_back({key, func});
        }
    }

    void Input::UnsetKeyUpCallback(uint32_t key)
    {
        for (auto iter = _key_up_callbacks.begin(); iter != _key_up_callbacks.end(); iter++)
        {
            if (iter->key == key)
            {
                _key_up_callbacks.erase(iter);
                break;
            }
        }
    }

    void Input::UnsetKeyDownCallback(uint32_t key)
    {
        for (auto iter = _key_down_callbacks.begin(); iter != _key_down_callbacks.end(); iter++)
        {
            if (iter->key == key)
            {
                _key_down_callbacks.erase(iter);
                break;
            }
        }

        _turbo_key &= ~key;
    }

    void Input::Poll(bool waiting)
    {
        SceCtrlData ctrl_data{0};
        if ((waiting ? sceCtrlReadBufferPositiveExt2(0, &ctrl_data, 1) : sceCtrlPeekBufferPositiveExt2(0, &ctrl_data, 1)) > 0)
        {
            uint32_t key = ctrl_data.buttons;
            key &= ~SCE_CTRL_HEADPHONE;
            if (ctrl_data.lx < (ANALOG_CENTER - ANALOG_THRESHOLD))
                key |= SCE_CTRL_LSTICK_LEFT;
            else if (ctrl_data.lx > (ANALOG_CENTER + ANALOG_THRESHOLD))
                key |= SCE_CTRL_LSTICK_RIGHT;

            if (ctrl_data.ly < (ANALOG_CENTER - ANALOG_THRESHOLD))
                key |= SCE_CTRL_LSTICK_UP;
            else if (ctrl_data.ly > (ANALOG_CENTER + ANALOG_THRESHOLD))
                key |= SCE_CTRL_LSTICK_DOWN;

            if (ctrl_data.rx < (ANALOG_CENTER - ANALOG_THRESHOLD))
                key |= SCE_CTRL_RSTICK_LEFT;
            else if (ctrl_data.rx > (ANALOG_CENTER + ANALOG_THRESHOLD))
                key |= SCE_CTRL_RSTICK_RIGHT;

            if (ctrl_data.ry < (ANALOG_CENTER - ANALOG_THRESHOLD))
                key |= SCE_CTRL_RSTICK_UP;
            else if (ctrl_data.ry > (ANALOG_CENTER + ANALOG_THRESHOLD))
                key |= SCE_CTRL_RSTICK_DOWN;

            _left_analog.x = ctrl_data.lx;
            _left_analog.y = ctrl_data.ly;
            _right_analog.x = ctrl_data.rx;
            _right_analog.y = ctrl_data.ry;

            key = _ProcTurbo(key);
            _ProcCallbacks(key);

            _last_key = key;
        }

        _front_touch.Poll();
        _rear_touch.Poll();
    }

    /*
    Up                        ______________________                      _______________
                             |                     |                     |
             _turbo_start_ms |  _turbo_interval_ms |  _turbo_interval_ms | ......
    Down   __________________|                     |_____________________|
    */
    uint32_t Input::_ProcTurbo(uint32_t key)
    {
        if (!_turbo_key)
        {
            return key;
        }

        uint64_t current = sceKernelGetProcessTimeWide();

        for (int i = 0; i < 32; i++)
        {
            uint32_t k = 1 << i;
            if (k & _turbo_key)
            {
                TurboKeyState *state = _turbo_key_states + i;

                if (k & key)
                {
                    if ((k & ~_last_key) && state->next_change_state_time == 0ll)
                    {
                        // first keydown
                        state->next_change_state_time = current + _turbo_start_ms;
                        state->down = true;
                    }
                    else if (current >= state->next_change_state_time)
                    {
                        // change state
                        state->next_change_state_time += _turbo_interval_ms;
                        state->down = !state->down;
                    }

                    if (!state->down)
                    {
                        key &= ~k;
                    }
                }
                else if (state->next_change_state_time != 0ll)
                {
                    state->next_change_state_time = 0ll;
                    state->down = false;
                }
            }
        }

        return key;
    }

    void Input::_ProcCallbacks(uint32_t key)
    {
        if (key != _last_key)
        {
            // LogDebug("key %08x", key);
            for (const auto &iter : _key_down_callbacks)
            {
                if (TEST_KEY(iter.key, key) && !TEST_KEY(iter.key, _last_key))
                {
                    // LogDebug("  call down: %08x %08x", iter.first, iter.second);
                    iter.func(this);
                    _current_hotkey |= iter.key;
                    break;
                }
            }

            if (_enable_key_up)
            {
                for (const auto &iter : _key_up_callbacks)
                {
                    // LogDebug("_key_up_callbacks %08x %08x", iter.first, _last_key);
                    if (TEST_KEY(iter.key, _last_key) && !TEST_KEY(iter.key, key) &&
                        (iter.key == _current_hotkey || !TEST_KEY(iter.key, _current_hotkey)))
                    {
                        // LogDebug("  call up: %08x %08x", iter.first, iter.second);
                        iter.func(this);
                        if (key)
                        {
                            _enable_key_up = false;
                        }
                        break;
                    }
                }
            }
            else if (key == 0)
            {
                _enable_key_up = true;
            }

            if (key == 0)
            {
                _current_hotkey = 0ull;
            }
        }
    }

    void Input::SetTurbo(uint32_t key)
    {
        _turbo_key |= key;
    }

    void Input::UnsetTurbo(uint32_t key)
    {
        _turbo_key &= ~key;
    }

    void Input::SetTurboInterval(uint64_t turbo_start, uint64_t turbo_interval)
    {
        _turbo_start_ms = turbo_start;
        _turbo_interval_ms = turbo_interval;
    }

    void Input::Reset()
    {
        _turbo_key = 0;
        _key_down_callbacks.clear();
        _key_up_callbacks.clear();
        _turbo_start_ms = DEFAULT_TURBO_START_TIME;
        _turbo_interval_ms = DEFAULT_TURBO_INTERVAL;
    }

    void Input::PushCallbacks()
    {
        LogDebug("PushCallbacks %08x", this);
        _callback_stack.push(_key_up_callbacks);
        _callback_stack.push(_key_down_callbacks);
        _key_down_callbacks.clear();
        _key_up_callbacks.clear();
    }

    void Input::PopCallbacks()
    {
        LogDebug("PopCallbacks %08x", this);
        _key_down_callbacks.clear();
        _key_up_callbacks.clear();
        if (_callback_stack.size() > 0)
        {
            _key_down_callbacks = _callback_stack.top();
            _callback_stack.pop();
        }
        else
        {
            LogWarn("_callback_stack is empty, can't pop to _key_down_callbacks");
        }

        if (_callback_stack.size() > 0)
        {
            _key_up_callbacks = _callback_stack.top();
            _callback_stack.pop();
        }
        else
        {
            LogWarn("_callback_stack is empty, can't pop to _key_up_callbacks");
        }
    }
}