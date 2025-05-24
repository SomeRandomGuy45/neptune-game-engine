#include "gameLoaderHelper.h"

namespace neptune {

bool isVaildGame(const std::string& gameFolderPath) {
    return (std::filesystem::exists(gameFolderPath + "/main.json") && std::filesystem::exists(gameFolderPath + "/config.json"));
}  


};