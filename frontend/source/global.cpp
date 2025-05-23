#include <string.h>
#include <libretro.h>
#include <psp2common/ctrl.h>
#include "input.h"
#include "global.h"
#include "config_types.h"

AppStatus gStatus;
bool gBootFromArch = false;
std::string gBootRomPath;
