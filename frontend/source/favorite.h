#pragma once
#include <map>
#include <string>
#include "directory.h"
#include "defines.h"

struct Favorite
{
    DirItem item;
    std::string path;
    std::string rom_name;

    bool operator<(const Favorite &other) const { return item.name < other.item.name; };
};

class Favorites : public std::map<std::string, Favorite>
{
public:
    Favorites();
    virtual ~Favorites();
    bool Load(const char *path = CORE_FAVOURITE_PATH);
    bool Save(const char *path = CORE_FAVOURITE_PATH);

private:
};

extern Favorites *gFavorites;