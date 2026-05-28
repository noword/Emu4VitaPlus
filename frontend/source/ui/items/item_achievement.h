#pragma once
#include "item_selectable.h"
#include "achievement.h"

class ItemAchievement : public ItemBase
{
public:
    ItemAchievement(Achievement *achievement);
    virtual ~ItemAchievement();

    virtual void Show(bool selected) override;

private:
    Achievement *_achievement;
};