#include "game.h"

int main(int argc, char **argv) {
    neptune::Game game;
    game.initLua();
    neptune::Box* boxTest = new neptune::Box(0, 0, 100, 100, SDL_Color{255, 255, 255, 255});
    boxTest->setColor(SDL_Color{0, 255, 0, 1});
    boxTest->name = "moooo";
    neptune::Triangle* triangleTest = new neptune::Triangle(100, 100, 100, 100, SDL_Color{255, 255, 255, 255});
    neptune::Circle* circleTest = new neptune::Circle(-100, 100, 100, SDL_Color{255, 255, 255, 255});
    neptune::Sprite* spriteTest = new neptune::Sprite("image.png", 100, 100, 100, 100);
    spriteTest->setColor(SDL_Color{255, 0, 0, 1});
    spriteTest->name = "hiiiii";
    neptune::Text* textTest = new neptune::Text(-100, -100, 100, 100, "hi!");
    textTest->name = "myhappyguy";
    neptune::EventListener* listener = new neptune::EventListener();
    listener->name = "listener";
    neptune::Audio* audio = new neptune::Audio("test.wav");
    audio->name = "audio";
    game.addBaseObject(std::unique_ptr<neptune::Audio>(audio));
    game.addBaseObject(std::unique_ptr<neptune::EventListener>(listener));
    game.addObject(std::unique_ptr<neptune::Box>(boxTest));
    game.addObject(std::unique_ptr<neptune::Triangle>(triangleTest));
    game.addObject(std::unique_ptr<neptune::Sprite>(spriteTest));
    game.addObject(std::unique_ptr<neptune::Circle>(circleTest));
    game.addObject(std::unique_ptr<neptune::Text>(textTest));
    game.init();
    return 0;
}