#include <game.h>
#include <memory>

void initTestGame(neptune::Game& game) {
    auto boxTest = std::make_unique<neptune::Box>(150, 0, 100, 100, SDL_Color{255, 255, 255, 255});
    boxTest->setColor(SDL_Color{0, 255, 0, 1});
    boxTest->name = "moooo";
    boxTest->setZIndex(1);
    auto triangleTest = std::make_unique<neptune::Triangle>(100, 100, 100, 100, SDL_Color{255, 255, 255, 255});
    triangleTest->setZIndex(2);
    auto circleTest = std::make_unique<neptune::Circle>(-100, 100, 100, SDL_Color{255, 255, 255, 255});
    auto spriteTest = std::make_unique<neptune::Sprite>("image.png", 100, 100, 100, 100);
    spriteTest->setColor(SDL_Color{255, 0, 0, 1});
    spriteTest->name = "hiiiii";
    spriteTest->setZIndex(3);
    auto textTest = std::make_unique<neptune::Text>(300, 100, 300, 300, "hello world!");
    textTest->name = "myhappyguy";
    auto listener = std::make_unique<neptune::EventListener>();
    listener->name = "listener";
    auto audio = std::make_unique<neptune::Audio>("test.wav");
    audio->name = "audio";
    game.addBaseObject(std::move(audio));
    game.addBaseObject(std::move(listener));
    game.addObject(std::move(boxTest));
    game.addObject(std::move(triangleTest));
    game.addObject(std::move(spriteTest));
    game.addObject(std::move(circleTest));
    game.addObject(std::move(textTest));
}

int runApp(int argc, char **argv) {
    neptune::debugFunction_01();
    neptune::fixActivationPolicyFunc();
    const char* filePath = neptune::getFileFromPicker();
    neptune::Game game;
    if (filePath == nullptr) {
        neptune::game_log("No file stuff was selected, or user is on unsupported platform", neptune::WARNING);
        game.loadGame_DEBUG("exampleProject.project", true);
    } else {
        neptune::game_log("File path: " + std::string(filePath));
        game.loadGame_DEBUG(filePath, false);
    }
    game.initLua();
    game.loadNewScene("scene1");
    //initTestGame(game);
    game.init();
    delete filePath;
    return 0;
}

#ifndef _WIN32

int main(int argc, char** argv) {
    return runApp(argc, argv);
}

#else

#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return runApp(__argc, __argv);
}

#endif