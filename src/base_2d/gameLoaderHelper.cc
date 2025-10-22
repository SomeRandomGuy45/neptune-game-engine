#include <gameLoaderHelper.h>

namespace neptune {

bool isVaildGame(const std::string& gameFolderPath) {
    if (gameFolderPath.empty()) return false;
    std::cout << gameFolderPath << "\n";
    return (std::filesystem::exists(gameFolderPath + "/main.json") && std::filesystem::exists(gameFolderPath + "/config.json") && std::filesystem::exists(gameFolderPath + "/assets"));
}  


};