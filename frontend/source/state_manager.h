#pragma once
#include <string>
#include <vita2d.h>
#include "file.h"

#define MAX_STATES 6
#define AUTO_STATE_INDEX 0
#define STATE_SCREENSHOT_HEIGHT 96

class State
{
public:
    State(const char *slot_name);
    virtual ~State();

    void Init(const char *game_name);
    const char *SlotName() const { return _slot_name.c_str(); };
    bool Save();
    bool Load();
    bool Remove();
    bool Valid() { return _valid; };
    const SceDateTime &CreateTime() { return _create_time; };
    vita2d_texture *Texture() { return _texture == nullptr ? _empty_texture : _texture; };

    friend class CoreStateManager;

private:
    bool _valid;
    std::string _slot_name;
    std::string _state_path;
    std::string _image_path;
    SceDateTime _create_time;
    vita2d_texture *_texture;
    static vita2d_texture *_empty_texture;
};

struct CoreStateManager
{
    CoreStateManager();
    virtual ~CoreStateManager();
    void Init(const char *name);
    State *GetNewest();
    State *states[MAX_STATES];
};
