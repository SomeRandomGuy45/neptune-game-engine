#include "game.h"

namespace neptune {
    void Game::init(const std::string& winName ) {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            game_log("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()), neptune::CRITICAL);
            exit(1);
        }
        window = SDL_CreateWindow("Neptune Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | flag_1 | flag_2);
        if(!window)
        {
            game_log("Window could not be created! SDL_Error: " + std::string(SDL_GetError()), neptune::CRITICAL);
            exit(1);
        }
        game_log("Made Window");
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer)
        {
            game_log("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()), neptune::CRITICAL);
            exit(1);
        }
        game_log("Made renderer");
        game_log("Game ready!");
        bool quit = false;
        while (!quit) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
            }
            render();
        }
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    void Game::render()
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        for (const auto& obj : objects) {
            obj->render(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        }
        SDL_RenderPresent(renderer);
    }
} // namespace neptune