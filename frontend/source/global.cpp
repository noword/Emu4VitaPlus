#include "global.h"

AppStatus gStatus;
bool gBootFromArch = false;
std::string gBootRomPath;

Emu4VitaPlus::VideoThread *gVideo = nullptr;
Overlays *gOverlays;
RetroArchPlaylists *gPlaylists;
Ui *gUi = nullptr;
Emulator *gEmulator = nullptr;
Emu4VitaPlus::Config *gConfig;
Favorites *gFavorites = nullptr;
CoreStateManager *gStateManager = nullptr;
Shaders *gShaders = nullptr;
RomNameMap *gRomNameMap = nullptr;
Network *gNetwork = nullptr;
RetroAchievements *gRetroAchievements = nullptr;
Hint *gHint = nullptr;
InputTextDialog *gInputTextDialog = nullptr;
Notifications *gNotifications = nullptr;