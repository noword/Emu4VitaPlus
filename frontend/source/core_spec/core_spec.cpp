#include <libretro.h>
#include "defines.h"
#include "core_spec.h"
#include "log.h"

const char *DEFAULT_CORE_SETTINGS[][2] =
// #if defined(FBNEO_BUILD)
//     {{"fbneo-cyclone", "enabled"},
//      {"fbneo-fm-interpolation", "disabled"},
//      {"fbneo-sample-interpolation", "disabled"},
//      {"fbneo-frameskip-type", "Auto"}};
#if defined(DOSBOX_PURE_BUILD)
    {{"dosbox_pure_cycles", "max"}};
#else
    {{}};
#endif

void DefaultCoreOptions(CoreOptions *core_options)
{
    for (size_t i = 0; i < sizeof(DEFAULT_CORE_SETTINGS) / sizeof(DEFAULT_CORE_SETTINGS[0]); i++)
    {
        if (DEFAULT_CORE_SETTINGS[i][0])
            core_options->emplace(DEFAULT_CORE_SETTINGS[i][0], CoreOption{DEFAULT_CORE_SETTINGS[i][1]});
    }
}
