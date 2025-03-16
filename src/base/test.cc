#include "game.h"

int main(int argc, char **argv) {
    neptune::Game game;
    neptune::Box* boxTest = new neptune::Box(0, 0, 20, 20, SDL_Color{255, 255, 255, 255});
    game.addObject(std::unique_ptr<neptune::Box>(boxTest));
    game.init();
    return 0;
}