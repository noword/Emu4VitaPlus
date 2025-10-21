#pragma once
#include "item_selectable.h"
#include "achievement.h"

class ItemAchievement : public ItemSelectable
{
public:
    ItemAchievement(Achievement *achievement);
    virutal ~ItemAchievement();

    virtual void Show(bool selected) override;

private:
    Achievement *_achievement;
}