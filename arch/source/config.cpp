#include "SimpleIni.h"
#include "config.h"
#include "file.h"
#include "log.h"
#include "utils.h"

#define MAIN_SECTION "MAIN"

Config *gConfig = nullptr;

Config::Config()
{
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

    tmp = ini.GetValue(MAIN_SECTION, "language", nullptr);
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
    else
    {
        language = Utils::GetDefaultLanguage();
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
    File::Remove(path);
    return ini.SaveFile(path, false) == SI_OK;
}