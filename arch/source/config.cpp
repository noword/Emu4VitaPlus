#include "SimpleIni.h"
#include "config.h"
#include "file.h"
#include "log.h"
#include "utils.h"

#define MAIN_SECTION "MAIN"
#define CONSOLE_SECTION "CONSOLE"

Config *gConfig = nullptr;

const char *CONSOLE_NAMES[] = {
    "ATARI2600",
    "ATARI5200",
    "ATARI7800",
    "C64",
    "VECTREX",
    "ZXS",
    "DOS",
    "PC98",
    "MSX",
    "NES",
    "AMIGA",
    "X68000",
    "PCE",
    "MD",
    "GBC",
    "SNES",
    "NEOCD",
    "PS1",
    "NGP",
    "WSC",
    "GBA",
    "ARC",
};

Config::Config()
{
    language = Utils::GetDefaultLanguage();
    Load();
}

Config::~Config()
{
    Save();
}

bool Config::Load(const char *path)
{
    LogFunctionName;
    LogDebug("path: %s", path);
    CSimpleIniA ini;
    if (ini.LoadFile(path) != SI_OK)
    {
        return false;
    }

    const char *tmp = ini.GetValue(MAIN_SECTION, "last_core");
    if (tmp)
    {
        last_core = tmp;
    }

    tmp = ini.GetValue(MAIN_SECTION, "language");
    if (tmp)
    {
        for (size_t i = 0; i < LANGUAGE_COUNT; i++)
        {
            if (strcmp(tmp, gLanguageNames[i]) == 0)
            {
                language = (LANGUAGE)i;
                break;
            }
        }
    }

    for (size_t i = 0; i < CONSOLE_COUNT; i++)
    {
        consoles[i] = ini.GetBoolValue(CONSOLE_SECTION, CONSOLE_NAMES[i], true);
    }

    return true;
}

bool Config::Save(const char *path)
{
    LogFunctionName;
    LogDebug("path: %s", path);

    CSimpleIniA ini;

    ini.SetValue(MAIN_SECTION, "last_core", last_core.c_str());
    ini.SetValue(MAIN_SECTION, "language", gLanguageNames[language]);

    for (size_t i = 0; i < CONSOLE_COUNT; i++)
    {
        ini.SetBoolValue(CONSOLE_SECTION, CONSOLE_NAMES[i], consoles[i]);
    }

    File::Remove(path);
    return ini.SaveFile(path, false) == SI_OK;
}