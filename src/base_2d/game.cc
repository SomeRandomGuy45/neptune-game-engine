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
        game_log("Testing lua!");
        main_lua_state.script_file("main.lua");
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
    void Game::initLua()
    {
        main_lua_state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os, sol::lib::table);
        main_lua_state.new_usertype<neptune::Object>("Object",
            "setName", [this](neptune::Object& obj, const std::string& newName) {
                auto it = workspace.objects.find(obj.name);
                if (it != workspace.objects.end()) {
                    auto objPtr = std::move(it->second);
                    workspace.objects.erase(it); 
                    obj.name = newName;
                    workspace.objects[newName] = std::move(objPtr);
                }
            }
        );      
        main_lua_state.new_usertype<Color>("Color",
            sol::constructors<Color(Uint8, Uint8, Uint8, Uint8)>(),
            "getR", &Color::getR, "setR", &Color::setR,
            "getG", &Color::getG, "setG", &Color::setG,
            "getB", &Color::getB, "setB", &Color::setB,
            "getA", &Color::getA, "setA", &Color::setA
        );        
        main_lua_state.new_usertype<neptune::Sprite>("Sprite",
            sol::constructors<neptune::Sprite(std::string, int, int, int, int)>(),
            "setColor", [](neptune::Object& self, Color color) {
                self.setColor(color.toSDL());
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );
        main_lua_state.new_usertype<neptune::Box>("Box",
            sol::constructors<neptune::Box(int, int, int, int, SDL_Color)>()
        );

        main_lua_state.new_usertype<neptune::Triangle>("Triangle",
            sol::constructors<neptune::Triangle(int, int, int, int, SDL_Color)>()
        );

        main_lua_state.new_usertype<neptune::Circle>("Circle",
            sol::constructors<neptune::Circle(int, int, int, SDL_Color)>()
        );
        main_lua_state.new_usertype<Workspace>("Workspace",
            "getObject", [this](Workspace& ws, const std::string& name) -> sol::object {
                auto objVariant = ws.getObject(name);
                if (objVariant) {
                    return std::visit([this](auto& obj) {
                        return sol::make_object(main_lua_state, obj.get());
                    }, *objVariant);
                }
                return sol::lua_nil;
            }
        );
        main_lua_state.new_usertype<Game>("Game",
            "Workspace", sol::property([](Game& g) -> Workspace& {
                return g.workspace;
            })
        );
        main_lua_state["game"] = this;
        game_log("Made Lua engine");
    }
    void Game::render()
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        for (const auto& [name, objVariant] : workspace.objects) {
            std::visit([this](auto& obj) {
                obj->render(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
            }, objVariant);
        }
        SDL_RenderPresent(renderer);
    }
} // namespace neptune