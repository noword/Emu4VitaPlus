#include <string.h>
#include <SimpleIni.h>
#include "core_options.h"
#include "log.h"
#include "file.h"

#define CORE_SECTION "CORE"

const std::vector<LanguageString> CoreOption::GetValues() const
{
    std::vector<LanguageString> _values;
    _values.reserve(values.size());
    for (const auto &v : values)
    {
        _values.emplace_back(v.label ? v.label : v.value);
    }

    return _values;
}

size_t CoreOption::GetValueIndex()
{
    size_t count = 0;
    for (const auto &v : values)
    {
        if ((v.value && value == v.value) || (v.label && value == v.label))
        {
            return count;
        }
        count++;
    }

    return 0;
}

void CoreOption::SetValueIndex(size_t index)
{
    value = values[index].value;
}

void CoreOptions::Load(retro_core_options_intl *options)
{
    LogFunctionName;
    const retro_core_option_definition *us = options->us;
    while (us && us->key)
    {
        _Load(us);
        us++;
    }
}

void CoreOptions::Load(retro_core_options_v2_intl *options)
{
    LogFunctionName;
    if (!options->us)
    {
        return;
    }

    const retro_core_option_v2_definition *us = options->us->definitions;
    while (us && us->key)
    {
        _Load(us);
        us++;
    }
}

void CoreOptions::Load(retro_core_option_definition *options)
{
    LogFunctionName;
    while (options && options->key)
    {
        _Load(options);
        options++;
    }
}

void CoreOptions::Load(retro_core_options_v2 *options)
{
    LogFunctionName;
    const retro_core_option_v2_definition *definitions = options->definitions;
    while (definitions && definitions->key)
    {
        _Load(definitions);
        definitions++;
    }
}

template <typename T>
void CoreOptions::_Load(const T *define)
{
    LogDebug("  key: %s", define->key);
    LogDebug("  desc: %s", define->desc);
    LogDebug("  info: %s", define->info);
    LogDebug("  default_value: %s", define->default_value);
    LogDebug("");

    static const char emptry_string[] = "";

    const auto &iter = this->find(define->key);
    CoreOption *option;
    if (iter == this->end())
    {
        option = &((*this)[define->key] = CoreOption{define->default_value,
                                                     define->desc ? define->desc : emptry_string,
                                                     define->info ? define->info : emptry_string,
                                                     define->default_value});
    }
    else
    {
        option = &(iter->second);
        option->desc = define->desc ? define->desc : emptry_string;
        option->info = define->info ? define->info : emptry_string;
        option->values.clear();
    }

    const retro_core_option_value *v = define->values;
    while (v->value)
    {
        option->values.emplace_back(*v);
        v++;
    }
}

void CoreOptions::Default()
{
    LogFunctionName;
    for (auto &iter : *this)
    {
        iter.second.Default();
    }
}

bool CoreOptions::Get(retro_variable *var)
{
    LogFunctionName;
    LogDebug("  key: %s", var->key);
    auto iter = this->find(var->key);
    if (iter == this->end())
    {
        var->value = NULL;
        LogWarn("  %s not found!", var->key);
        return false;
    }
    else
    {
        var->value = iter->second.value.c_str();
        LogDebug("  value: %s", var->value);
        return true;
    }
}

bool CoreOptions::Load(const char *path)
{
    LogFunctionName;
    CSimpleIniA ini;
    if (ini.LoadFile(path) != SI_OK)
    {
        LogWarn("Load %s failed", path);
        return false;
    }

    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys(CORE_SECTION, keys);
    for (auto const &key : keys)
    {
        const char *value = ini.GetValue(CORE_SECTION, key.pItem, "NULL");
        this->emplace(key.pItem, CoreOption{value});
        LogDebug("  %s = %s", key.pItem, value);
    }

    return true;
}

bool CoreOptions::Save(const char *path)
{
    LogFunctionName;
    CSimpleIniA ini;
    for (auto const &iter : *this)
    {
        const char *key = iter.first.c_str();
        const CoreOption *option = &iter.second;
        ini.SetValue(CORE_SECTION, key, option->value.c_str());
    }

    return ini.SaveFile(path, false) == SI_OK;
}

void CoreOptions::SetVisable(const struct retro_core_option_display *option_display)
{
    LogFunctionName;
    auto iter = this->find(option_display->key);
    if (iter != this->end())
    {
        LogDebug("  %s %d", option_display->key, option_display->visible);
        iter->second.visible = option_display->visible;
    }
}