#include "game.h"

int main(int argc, char **argv) {
    neptune::Game game;
    game.initLua();
    neptune::Box* boxTest = new neptune::Box(0, 0, 100, 100, SDL_Color{255, 255, 255, 255});
    neptune::Triangle* triangleTest = new neptune::Triangle(100, 100, 100, 100, SDL_Color{255, 255, 255, 255});
    //neptune::Circle* circleTest = new neptune::Circle(-100, 100, 100, SDL_Color{255, 255, 255, 255});
    neptune::Sprite* spriteTest = new neptune::Sprite("image.png", 100, 100, 100, 100);
    spriteTest->setColor(SDL_Color{255, 0, 0, 1});
    spriteTest->name = "hiiiii";
    game.addObject(std::unique_ptr<neptune::Box>(boxTest));
    game.addObject(std::unique_ptr<neptune::Triangle>(triangleTest));
    game.addObject(std::unique_ptr<neptune::Sprite>(spriteTest));
    game.init();
    return 0;
}