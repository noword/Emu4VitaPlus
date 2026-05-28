#include <imgui_vita2d/imgui_vita.h>
#include "item_config.h"
#include "my_imgui.h"
#include "global.h"
#include "log.h"
#include "utils.h"

ItemConfig::ItemConfig(LanguageString text,
                       LanguageString info,
                       uint32_t *config,
                       std::vector<LanguageString> config_texts,
                       CallbackFunc active_callback,
                       CallbackFunc option_callback)
    : ItemSelectable(text, info, active_callback, option_callback),
      _config(config),
      _config_texts(std::move(config_texts))
{
    if (_config_texts.size() > 0 && *_config >= _config_texts.size())
    {
        *_config = 0;
    }
}

ItemConfig::~ItemConfig()
{
}

void ItemConfig::_OnClick(Input *input)
{
    LogFunctionName;

    ItemSelectable::_OnClick(input);
    if (_text.GetId() == LANG_INDEPENDENT_CONFIG)
    {
        gConfig->Save(CORE_CONFIG_PATH);
    }
    else
    {
        gConfig->Save();
    }
}

ItemIntConfig::ItemIntConfig(LanguageString text,
                             LanguageString info,
                             uint32_t *value,
                             size_t start,
                             size_t end,
                             size_t step,
                             CallbackFunc active_callback,
                             CallbackFunc option_callback)
    : ItemConfig(text, info, &_index, {}, active_callback, option_callback),
      _value(value),
      _step(step)
{
    _config_texts.reserve(end - start + 1);
    for (size_t i = start; i <= end; i += step)
    {
        _config_texts.emplace_back(LanguageString(std::to_string(i)));
    }

    _index = (*value - start) / step;
}

void ItemIntConfig::_SetIndex(size_t index)
{
    ItemConfig::_SetIndex(index);
    *_value = std::stoi(_config_texts[index].Get());
}