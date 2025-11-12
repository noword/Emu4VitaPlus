#include <imgui_vita2d/imgui_impl_vita2d.h>
#include "emulator.h"
#include "global.h"
#include "video.h"
#include "state_manager.h"
#include "config.h"
#include "ui.h"

void InputPollCallback()
{
    LogFunctionNameLimited;
}

int16_t InputStateCallback(unsigned port, unsigned device, unsigned index, unsigned id)
{
    LogFunctionNameLimited;

    // LogDebug("port:%d device:%d index:%d id:%d", port, device, index, id);

    if ((device == RETRO_DEVICE_JOYPAD || device == RETRO_DEVICE_ANALOG) && port != 0)
        return 0;

    switch (device)
    {
    case RETRO_DEVICE_JOYPAD:
        return gEmulator->_GetJoypadState(index, id);

    case RETRO_DEVICE_MOUSE:
        return gEmulator->_GetMouseState(index, id);

    case RETRO_DEVICE_KEYBOARD:
        if (ENABLE_KEYBOARD && gEmulator->_keyboard->Visable())
        {
            return gEmulator->_GetKeybaordState(index, id);
        }
        return 0;

    case RETRO_DEVICE_ANALOG:
        return gEmulator->_GetAnalogState(index, id);

    case RETRO_DEVICE_LIGHTGUN:
        return gEmulator->_GetLightGunState(index, id);

    case RETRO_DEVICE_POINTER:
        return gEmulator->_GetPointerState(index, id);

    default:
        return 0;
    }
}

bool SetSensorStateCallback(unsigned port, enum retro_sensor_action action, unsigned rate)
{
    LogFunctionName;

    LogDebug("  port: %d action: %d rate: %d", port, action, rate);

    switch (action)
    {
    case RETRO_SENSOR_ACCELEROMETER_DISABLE:
    case RETRO_SENSOR_GYROSCOPE_DISABLE:
        gEmulator->_motion_sensor.Stop();
        return true;

    case RETRO_SENSOR_ACCELEROMETER_ENABLE:
    case RETRO_SENSOR_GYROSCOPE_ENABLE:
        gEmulator->_motion_sensor.Start();
        return true;

    case RETRO_SENSOR_ILLUMINANCE_DISABLE:
        return true;

    default:
        return false;
    }
}

float SensorGetInputCallback(unsigned port, unsigned id)
{
    if (!gEmulator->_motion_sensor.Enabled())
    {
        return 0.f;
    }

    const SceMotionSensorState &state = gEmulator->_motion_sensor.GetState();
    switch (id)
    {
    case RETRO_SENSOR_ACCELEROMETER_X:
        return state.accelerometer.x;
    case RETRO_SENSOR_ACCELEROMETER_Y:
        return state.accelerometer.y;
    case RETRO_SENSOR_ACCELEROMETER_Z:
        return state.accelerometer.z;
    case RETRO_SENSOR_GYROSCOPE_X:
        return state.gyro.x;
    case RETRO_SENSOR_GYROSCOPE_Y:
        return state.gyro.y;
    case RETRO_SENSOR_GYROSCOPE_Z:
        return state.gyro.z;
    }

    return 0.f;
}

int16_t Emulator::_GetJoypadState(unsigned index, unsigned id)
{
    uint32_t key_states = _input.GetKeyStates();
    auto touch = _input.GetRearTouch();
    if (gConfig->sim_button & touch->GetState() == TouchDown)
    {
        const auto axis = touch->GetAxis();
        const auto center = touch->GetCenter();
        static const uint32_t button_map[] = {SCE_CTRL_L2, SCE_CTRL_R2, SCE_CTRL_L3, SCE_CTRL_L3};
        int flag = (axis.y < center.y ? 0 : 2) | (axis.x < center.x ? 0 : 1);
        key_states |= button_map[flag];
    }

    if (id == RETRO_DEVICE_ID_JOYPAD_MASK)
    {
        int16_t state = 0;
        int16_t key = 1;
        for (size_t i = 0; i < 16; i++)
        {
            if (key_states & _keys[i])
            {
                state |= key;
            }
            key <<= 1;
        }

        return state;
    }
    // else if (id >= 16)
    // {
    //     LogError("  InputStateCallback, wrong id %d", id);
    //     return 0;
    // }
    else
    {
        return (key_states & _keys[id & 0xf]) ? 1 : 0;
    }
}

int16_t Emulator::_GetAnalogState(unsigned index, unsigned id)
{
    if (_video_rotation == VIDEO_ROTATION_0 || _video_rotation == VIDEO_ROTATION_180)
    {
        if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT)
        {
            return id == RETRO_DEVICE_ID_ANALOG_X ? _input.GetMapedLeftAnalogX() : _input.GetMapedLeftAnalogY();
        }
        else
        {
            return id == RETRO_DEVICE_ID_ANALOG_X ? _input.GetMapedRightAnalogX() : _input.GetMapedRightAnalogY();
        }

        // const AnalogAxis aa = index == RETRO_DEVICE_INDEX_ANALOG_LEFT ? _input.GetLeftAnalogAxis() : _input.GetRightAnalogAxis();
        // return id == RETRO_DEVICE_ID_ANALOG_X ? ANALOG_PSV_TO_RETRO(aa.x) : ANALOG_PSV_TO_RETRO(aa.y);
    }
    else
    {
        if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT)
        {
            return id == RETRO_DEVICE_ID_ANALOG_X ? _input.GetReverseMapedRightAnalogY() : _input.GetMapedRightAnalogX();
        }
        else
        {
            return id == RETRO_DEVICE_ID_ANALOG_X ? _input.GetReverseMapedLeftAnalogY() : _input.GetMapedLeftAnalogX();
        }
        // const AnalogAxis aa = index == RETRO_DEVICE_INDEX_ANALOG_LEFT ? _input.GetRightAnalogAxis() : _input.GetLeftAnalogAxis();
        // return id == RETRO_DEVICE_ID_ANALOG_X ? -(ANALOG_PSV_TO_RETRO(aa.y) + 1) : ANALOG_PSV_TO_RETRO(aa.x);
    }
}

int16_t Emulator::_GetMouseState(unsigned index, unsigned id)
{
    if (gConfig->mouse == CONFIG_MOUSE_DISABLE)
    {
        return 0;
    }

    Touch *touch = gConfig->mouse == CONFIG_MOUSE_FRONT ? _input.GetFrontTouch() : _input.GetRearTouch();
    if (touch->IsEnabled())
    {
        switch (id)
        {
        case RETRO_DEVICE_ID_MOUSE_X:
            return touch->GetRelativeMovingX();

        case RETRO_DEVICE_ID_MOUSE_Y:
            return touch->GetRelativeMovingY();

        case RETRO_DEVICE_ID_MOUSE_LEFT:
            return (_input.GetKeyStates() & _keys[RETRO_DEVICE_ID_JOYPAD_L]) ? 1 : 0;

        case RETRO_DEVICE_ID_MOUSE_RIGHT:
            return (_input.GetKeyStates() & _keys[RETRO_DEVICE_ID_JOYPAD_R]) ? 1 : 0;

        default:
            break;
        }
    }
    return 0;
}

int16_t Emulator::_GetLightGunState(unsigned index, unsigned id)
{
    // LogDebug("index:%d id:%d", index, id);
    if (gConfig->lightgun == 0)
    {
        return 0;
    }

    Touch *front = _input.GetFrontTouch();
    if (!front->IsEnabled())
    {
        return 0;
    }

    switch (id)
    {
    case RETRO_DEVICE_ID_LIGHTGUN_TRIGGER:
        return front->GetState() == TouchUp ? 1 : 0;

    case RETRO_DEVICE_ID_LIGHTGUN_AUX_A:
        return (_input.GetKeyStates() & _keys[RETRO_DEVICE_ID_JOYPAD_A]) ? 1 : 0;

    case RETRO_DEVICE_ID_LIGHTGUN_AUX_B:
        return (_input.GetKeyStates() & _keys[RETRO_DEVICE_ID_JOYPAD_B]) ? 1 : 0;

    case RETRO_DEVICE_ID_LIGHTGUN_START:
        return (_input.GetKeyStates() & _keys[RETRO_DEVICE_ID_JOYPAD_START]) ? 1 : 0;

    case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X:
        return front->GetMapedX(_video_rect);

    case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y:
        return front->GetMapedY(_video_rect);

    case RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
    {
        const TouchAxis &axis = front->GetAxis();
        return !_video_rect.Contains(axis.x, axis.y);
    }

    case RETRO_DEVICE_ID_LIGHTGUN_RELOAD:
    {
        const TouchAxis &axis = front->GetAxis();
        return (!_video_rect.Contains(axis.x, axis.y)) && front->GetState() == TouchUp;
    }

    default:
        break;
    }

    return 0;
}

int16_t Emulator::_GetPointerState(unsigned index, unsigned id)
{
    Touch *front = _input.GetFrontTouch();
    if (!front->IsEnabled())
    {
        return 0;
    }

    switch (id)
    {
    case RETRO_DEVICE_ID_POINTER_X:
        return front->GetMapedX(_video_rect);

    case RETRO_DEVICE_ID_POINTER_Y:
        return front->GetMapedY(_video_rect);

    case RETRO_DEVICE_ID_POINTER_PRESSED:
        return front->GetState() == TouchDown ? 1 : 0;

    case RETRO_DEVICE_ID_POINTER_COUNT:
        return front->GetId();

    default:
        break;
    }

    return 0;
}

int16_t Emulator::_GetKeybaordState(unsigned index, unsigned id)
{
    LogFunctionNameLimited;
    return _keyboard->CheckKey((retro_key)id);
}

void Emulator::SetupKeys()
{
    LogFunctionName;
    if ((gStatus.Get() & (APP_STATUS_RUN_GAME | APP_STATUS_SHOW_UI_IN_GAME)) == 0)
    {
        return;
    }

    memset(_keys, 0, sizeof(_keys));
    _keys_mask = 0;
    _input.Reset();
    for (const auto &k : gConfig->control_maps)
    {
        for (const auto &retro : k.retros)
        {
            if (retro == 0xff)
            {
                break;
            }

            if (retro >= 16)
            {
                LogError("  wrong key config: %d %08x", retro, k.psv);
                continue;
            }

            if ((!gConfig->auto_rotating) || _video_rotation == VIDEO_ROTATION_0 || _video_rotation == VIDEO_ROTATION_180)
            {
                _keys[retro] |= k.psv;
            }
            else
            {
                switch (retro)
                {
                case RETRO_DEVICE_ID_JOYPAD_UP:
                    _keys[RETRO_DEVICE_ID_JOYPAD_RIGHT] |= k.psv;
                    break;
                case RETRO_DEVICE_ID_JOYPAD_DOWN:
                    _keys[RETRO_DEVICE_ID_JOYPAD_LEFT] |= k.psv;
                    break;
                case RETRO_DEVICE_ID_JOYPAD_LEFT:
                    _keys[RETRO_DEVICE_ID_JOYPAD_UP] |= k.psv;
                    break;
                case RETRO_DEVICE_ID_JOYPAD_RIGHT:
                    _keys[RETRO_DEVICE_ID_JOYPAD_DOWN] |= k.psv;
                    break;
                default:
                    _keys[retro] |= k.psv;
                    break;
                }
            }

            _keys_mask |= k.psv;
            if (k.turbo)
            {
                _input.SetTurbo(k.psv);
                LogDebug("_input.SetTurbo: %08x", k.psv);
            }
        }
    }

#define BIND_HOTKEY(KEY, FUNC)                                                                   \
    _input.SetKeyDownCallback(gConfig->hotkeys[KEY], std::bind(&Emulator::FUNC, this, &_input)); \
    LogDebug("SetKeyDownCallback " #FUNC " %08x", gConfig->hotkeys[KEY]);

#define BIND_HOTKEY_UP(KEY, FUNC)                                                              \
    _input.SetKeyUpCallback(gConfig->hotkeys[KEY], std::bind(&Emulator::FUNC, this, &_input)); \
    LogDebug("SetKeyUpCallback " #FUNC " %08x", gConfig->hotkeys[KEY]);

    BIND_HOTKEY(SAVE_STATE, _OnHotkeySave);
    BIND_HOTKEY(LOAD_STATE, _OnHotkeyLoad);
    BIND_HOTKEY(EXIT_GAME, _OnHotkeyExitGame);
    BIND_HOTKEY(GAME_SPEED_UP, _OnHotkeySpeedUp);
    BIND_HOTKEY(GAME_SPEED_DOWN, _OnHotkeySpeedDown);
    BIND_HOTKEY(GAME_REWIND, _OnHotkeyRewind);
    // BIND_HOTKEY(CONTROLLER_PORT_UP, _OnHotkeyCtrlPortUp);
    // BIND_HOTKEY(CONTROLLER_PORT_DOWN, _OnHotkeyCtrlPortDown);

    if (ENABLE_KEYBOARD)
    {
        BIND_HOTKEY(SWITCH_KEYBOARD, _OnHotkeyKeyboard);
        BIND_HOTKEY(KEYBOARD_UP, _OnHotkeyKeyboardUp);
        BIND_HOTKEY(KEYBOARD_DOWN, _OnHotkeyKeyboardDown);
    }

    BIND_HOTKEY_UP(GAME_REWIND, _OnHotkeyRewindUp);
    BIND_HOTKEY_UP(MENU_TOGGLE, _OnPsButton);

    _input.SetTurboInterval(DEFAULT_TURBO_START_TIME, 20000);

    _input.GetFrontTouch()->Enable(gConfig->FrontEnabled());
    _input.GetRearTouch()->Enable(gConfig->RearEnabled());

    int count = 0;
    for (const auto &device : gConfig->device_options)
    {
        device.Apply(count++);
    }
}

void Emulator::SetupKeysWithSaveConfig()
{
    SetupKeys();
    gConfig->Save();
}

void Emulator::_OnPsButton(Input *input)
{
    LogFunctionName;
    if (_keyboard && _keyboard->Visable())
    {
        gVideo->Lock();
        _keyboard->SetVisable(false);
        gVideo->Unlock();
    }
    gStatus.Set(APP_STATUS_SHOW_UI_IN_GAME);
    Save();
}

void Emulator::_OnHotkeySave(Input *input)
{
    LogFunctionName;

    if (gRetroAchievements->GetHardcoreEnabled())
        return;

    Lock();
    gStateManager->states[0]->Save();
    Unlock();
}

void Emulator::_OnHotkeyLoad(Input *input)
{
    LogFunctionName;

    if (gRetroAchievements->GetHardcoreEnabled())
        return;

    Lock();
    gStateManager->states[0]->Load();
    Unlock();
}

void Emulator::_OnHotkeySpeedUp(Input *input)
{
    LogFunctionName;
    if (_speed < MAX_SPEED)
    {
        float step = gConfig->GetSpeedStep();
        double new_speed;
        if (step > 0.)
        {
            new_speed = _speed + step;
            if (new_speed > MAX_SPEED)
            {
                new_speed = MAX_SPEED;
            }
            else if (_speed < 1.0 && new_speed > 1.0)
            {
                new_speed = 1.0;
            }
        }
        else
        {
            new_speed = (_speed >= 1.0 ? MAX_SPEED : 1.0);
        }

        SetSpeed(new_speed);

        _ShowSpeedHint();
    }
}

void Emulator::_OnHotkeySpeedDown(Input *input)
{
    LogFunctionName;

    if (_speed > MIN_SPEED)
    {
        float step = gConfig->GetSpeedStep();
        double new_speed;
        if (step > 0.)
        {
            new_speed = _speed - step;
            if (new_speed < MIN_SPEED)
            {
                new_speed = MIN_SPEED;
            }
            else if (_speed > 1.0 && new_speed < 1.0)
            {
                new_speed = 1.0;
            }
        }
        else
        {
            new_speed = (_speed <= 1.0 ? MIN_SPEED : 1.0);
        }

        if (gRetroAchievements->GetHardcoreEnabled() && new_speed < 1.0)
        {
            new_speed = 1.0;
        }

        SetSpeed(new_speed);

        _ShowSpeedHint();
    }
}

void Emulator::_OnHotkeyRewind(Input *input)
{
    if (gConfig->rewind && (!gRetroAchievements->GetHardcoreEnabled()))
    {
        gStatus.Set(APP_STATUS_REWIND_GAME);
    }
}

void Emulator::_OnHotkeyRewindUp(Input *input)
{
    if (gConfig->rewind)
    {
        gStatus.Set(APP_STATUS_RUN_GAME);
        _rewind_manager.Signal();
    }
}

void Emulator::_OnHotkeyExitGame(Input *input)
{
    LogFunctionName;
    UnloadGame();
}

void Emulator::_OnHotkeyKeyboard(Input *input)
{
    LogFunctionName;

    if (_keyboard)
    {
        gVideo->Lock();
        _keyboard->SetVisable(!_keyboard->Visable());
        gVideo->Unlock();
    }
}

void Emulator::_OnHotkeyKeyboardUp(Input *input)
{
    LogFunctionName;
    if (_keyboard)
    {
        gVideo->Lock();
        _keyboard->SetKeyboardUp();
        gVideo->Unlock();
    }
}

void Emulator::_OnHotkeyKeyboardDown(Input *input)
{
    LogFunctionName;
    if (_keyboard)
    {
        gVideo->Lock();
        _keyboard->SetKeyboardDown();
        gVideo->Unlock();
    }
}

void Emulator::_SetControllerInfo(retro_controller_info *info)
{
    LogFunctionName;

    gConfig->device_options.Load(info);
    gUi->UpdateControllerOptions();
}