#pragma once
#include <vector>
#include <stdint.h>
#include "language_string.h"
#include "item_selectable.h"

class ItemConfig : public ItemSelectable
{
public:
    ItemConfig(LanguageString text,
               LanguageString info,
               uint32_t *config,
               std::vector<LanguageString> config_texts,
               CallbackFunc active_callback = nullptr,
               CallbackFunc option_callback = nullptr);

    virtual ~ItemConfig();

protected:
    std::vector<LanguageString> _config_texts;
    uint32_t *_config;

    virtual void _OnClick(Input *input) override;
    virtual size_t _GetTotalCount() override { return _config_texts.size(); };
    virtual const char *_GetOptionString(size_t index) override { return _config_texts[index].Get(); };
    virtual size_t _GetIndex() override { return *_config; };
    virtual void _SetIndex(size_t index) override { *_config = index; };
};

class ItemIntConfig : public ItemConfig
{
public:
    ItemIntConfig(LanguageString text,
                  LanguageString info,
                  uint32_t *value,
                  size_t start,
                  size_t end,
                  size_t step = 1,
                  CallbackFunc active_callback = nullptr,
                  CallbackFunc option_callback = nullptr);

    virtual void _SetIndex(size_t index) override;

private:
    size_t _step;
    uint32_t *_value;
    uint32_t _index;
};