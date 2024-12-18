#include <string.h>
#include "core_options.h"
#include "log.h"
#include "file.h"
#include "utils.h"

#define CORE_SECTION "CORE"

const std::vector<LanguageString> CoreOption::GetValues() const
{
    std::vector<LanguageString> _values;
    _values.reserve(values.size());
    for (const auto &v : values)
    {
        _values.emplace_back(v.label.empty() ? v.value : v.label);
    }

    return _values;
}

size_t CoreOption::GetValueIndex()
{
    size_t count = 0;
    for (const auto &v : values)
    {
        if (value == v.value || value == v.label)
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

void CoreOptions::Load(retro_variable *variables)
{
    char *value_cp = new char[512];
    size_t value_cp_size = 512;

    while (variables && variables->key && variables->value)
    {
        LogDebug("  key:   %s", variables->key);
        LogDebug("  value: %s", variables->value);

        size_t vsize = strlen(variables->value);
        if (vsize >= value_cp_size)
        {
            delete[] value_cp;
            value_cp_size = vsize + 1;
            value_cp = new char[value_cp_size];
        }

        strcpy(value_cp, variables->value);

        char *desc = value_cp;
        char *values = strchr(value_cp, ';');
        *values = '\x00';
        values += 2;
        char *p = strchr(values, '|');
        *p = '\x00';
        p++;

        // example:
        // "Frameskip; 0|1|2|3|4|5"
        // desc ==> "Frameskip"
        // values ==> "0"  // default value
        // p ==> "1|2|3|4|5""

        CoreOption *option;
        const auto &iter = this->find(variables->key);
        if (iter == this->end())
        {
            option = &((*this)[variables->key] = CoreOption{values,
                                                            desc,
                                                            "",
                                                            values});
        }
        else
        {
            option = &(iter->second);
            option->desc = desc;
            option->default_value = values;
            option->values.clear();
        }

        do
        {
            option->values.push_back({values, ""});
            values = p;
            p = strchr(p, '|');
            if (p)
            {
                *p = '\x00';
                p++;
            }
            else
            {
                option->values.push_back({values, ""});
                break;
            }
        } while (true);

        variables++;
    }

    delete[] value_cp;
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

    const auto &iter = this->find(define->key);
    CoreOption *option;
    if (iter == this->end())
    {
        option = &((*this)[define->key] = CoreOption{define->default_value,
                                                     define->desc ? define->desc : "",
                                                     define->info ? define->info : "",
                                                     define->default_value});
    }
    else
    {
        option = &(iter->second);
        option->desc = define->desc ? define->desc : "";
        option->info = define->info ? define->info : "";
        option->default_value = define->default_value;
        option->values.clear();
    }

    Utils::TrimString(&option->info);

    const retro_core_option_value *v = define->values;
    bool checked = false;
    while (v->value)
    {
        if (!checked && option->value == v->value)
        {
            checked = true;
        }
        option->values.push_back({v->value, v->label ? v->label : ""});
        v++;
    }

    if (!checked)
    {
        option->Default();
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
    if (!(var && var->key))
    {
        return false;
    }

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

bool CoreOptions::Load(CSimpleIniA &ini)
{
    LogFunctionName;

    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys(CORE_SECTION, keys);
    for (auto const &key : keys)
    {
        const auto &iter = this->find(key.pItem);
        const char *value = ini.GetValue(CORE_SECTION, key.pItem, "NULL");
        LogDebug("  %s = %s", key.pItem, value);

        if (iter == this->end())
        {
            this->emplace(key.pItem, CoreOption{value});
        }
        else
        {
            iter->second.value = value;
        }
    }

    return true;
}

bool CoreOptions::Save(CSimpleIniA &ini)
{
    for (auto const &iter : *this)
    {
        const char *key = iter.first.c_str();
        const CoreOption *option = &iter.second;
        ini.SetValue(CORE_SECTION, key, option->value.c_str());
    }
    return true;
}

bool CoreOptions::Load(const char *path)
{
    LogFunctionName;
    LogDebug("path: %s", path);

    CSimpleIniA ini;
    if (ini.LoadFile(path) != SI_OK)
    {
        return false;
    }

    return Load(ini);
}

bool CoreOptions::Save(const char *path)
{
    LogFunctionName;
    LogDebug("path: %s", path);

    CSimpleIniA ini;

    if (Save(ini))
    {
        File::Remove(path);
        return ini.SaveFile(path, false) == SI_OK;
    }
    else
    {
        return false;
    }
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