#include "device_options.h"
#include "retro_module.h"
#include "icons.h"
#include "log.h"

#define DEVCIE_SECTION "DEVICE"
static const char *Unset = gTexts[LANGUAGE_ENGLISH][LANG_UNSET];

// the order is RETRO_DEVICE_xxx
static const char *DEVICE_ICON[] = {
    "",
    ICON_GAMEPAD_0_SPACE,
    ICON_MOUSE_SPACE,
    ICON_KEYBOARD_SPACE,
    ICON_LIGHTGUN_SPACE,
    ICON_JOYSTICK_SPACE,
    ICON_TOUCH_SPACE};

const std::vector<LanguageString> ControllerTypes::GetValues() const
{
    std::vector<LanguageString> _values;
    _values.reserve(this->size() + 1);
    _values.emplace_back(TEXT(LANG_UNSET));
    for (const auto &v : *this)
    {
        uint32_t base = v.device & 0xff;
        if (base >= sizeof(DEVICE_ICON) / sizeof(DEVICE_ICON[0]))
        {
            base = 0;
        }
        _values.emplace_back(std::string(DEVICE_ICON[base]) + LanguageString(v.desc).Get());
    }

    return _values;
}

size_t ControllerTypes::GetValueIndex()
{
    for (size_t i = 0; i < this->size(); i++)
    {
        if (value == (*this)[i].desc)
        {
            return i + 1;
        }
    }

    return 0;
}

void ControllerTypes::SetValueIndex(size_t index)
{
    if (index == 0 || index > this->size())
    {
        value = Unset;
    }
    else
    {
        value = (*this)[index - 1].desc;
    }

    LogDebug("SetValueIndex %d %s", index, value.c_str());
}

void ControllerTypes::Default()
{
    value = Unset;
}

void ControllerTypes::Apply(uint32_t port) const
{
    LogFunctionName;

    for (const auto &v : *this)
    {
        if (value == v.desc)
        {
            LogDebug("  retro_set_controller_port_device: %d %x", port, v.device);
            retro_set_controller_port_device(port, v.device);
            break;
        }
    }
}

void DeviceOptions::Load(retro_controller_info *info)
{
    LogFunctionName;
    unsigned count = 0;
    while (info->types)
    {
        ControllerTypes *types;
        if (count >= this->size())
        {
            this->push_back(ControllerTypes{});
        }
        types = &(*this)[count];
        types->clear();

        bool found = false;
        for (unsigned i = 0; i < info->num_types && info->types[i].desc; i++)
        {
            LogDebug(" %d %08x %s", count, info->types[i].id, info->types[i].desc);
            types->push_back(ControllerType{info->types[i].desc, info->types[i].id});
            if (types->value == info->types[i].desc)
            {
                found = true;
            }
        }
        if (!found)
            types->value = Unset;

        info++;
        count++;
    }
}

bool DeviceOptions::Load(CSimpleIniA &ini)
{
    LogFunctionName;
    char tmp[32];
    int num_types = ini.GetLongValue(DEVCIE_SECTION, "num_types");
    if (num_types == 0)
        return false;

    this->clear();
    this->reserve(num_types);
    for (int i = 0; i < num_types; i++)
    {
        snprintf(tmp, 32, "device_%02d", i);
        ControllerTypes types;
        types.value = ini.GetValue(DEVCIE_SECTION, tmp, Unset);
        this->emplace_back(types);
    }

    return true;
}

bool DeviceOptions::Save(CSimpleIniA &ini)
{
    char tmp[32];
    int count = 0;
    for (auto const &iter : *this)
    {
        if (iter.value != Unset)
        {
            snprintf(tmp, 32, "device_%02d", count);
            ini.SetValue(DEVCIE_SECTION, tmp, iter.value.c_str());
        }
        count++;
    }
    ini.SetLongValue(DEVCIE_SECTION, "num_types", count);

    return true;
}