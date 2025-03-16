#include <vector>
#include <memory>

#include "objects.h"
#include "helper.h"

namespace neptune {

    class Game {
    public:
        void init(const std::string& winName = "Neptune Game");
        void addObject(std::unique_ptr<Object> obj) { objects.push_back(std::move(obj)); }
        int SCREEN_WIDTH = 640;
        int SCREEN_HEIGHT = 480;
        // SDL_WINDOW_FULLSCREEN and SDL_WINDOW_RESIZABLE are the only ones we can use
        Uint32 flag_1 = 0;
        Uint32 flag_2 = 0;
    private:
        void render();
        SDL_Window* window;
        SDL_Renderer* renderer;
        std::vector<std::unique_ptr<Object>> objects;
    };

} // namespace neptune