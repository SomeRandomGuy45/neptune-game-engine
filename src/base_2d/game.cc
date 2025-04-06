#include "game.h"

namespace neptune {
    void Game::init(const std::string& winName ) {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            game_log("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()), neptune::CRITICAL);
            exit(1);
        }
        if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 1024) < 0) {
            game_log("SDL_Mixer could not initialize! SDL_Error: " + std::string(Mix_GetError()), neptune::CRITICAL);
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
        std::thread luaScriptThread([this](){
            main_lua_state.script_file("main.lua");
        });
        luaScriptThread.detach();
        bool quit = false;
        while (!quit) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    for (const auto& [name, objVariant] : workspace.objects) {
                        bool isClicked = false;
                        std::visit([this, mouseX, mouseY, name, &isClicked](auto& obj) {
                            isClicked = obj->isClicked(mouseX, mouseY, SCREEN_WIDTH, SCREEN_HEIGHT);
                        }, objVariant);
                        if (isClicked) {
                            std::visit([this](auto& obj) {
                                obj->DoEventCallback(MOUSE);
                            }, objVariant);
                        }
                    }
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
                auto range = workspace.objects.equal_range(obj.name);
                for (auto it = range.first; it != range.second; ++it) {
                    auto objPtr = std::move(it->second);
                    workspace.objects.erase(it);
                    workspace.objects.emplace(newName, std::move(objPtr));
                    break;
                }
                obj.name = newName; 
            }
        );
        
        main_lua_state.new_usertype<neptune::BaseObject>("BaseObject", 
            "setName", [this](neptune::BaseObject& obj, const std::string& newName) {
                auto range = workspace.objects_base.equal_range(obj.name);
                for (auto it = range.first; it != range.second; ++it) {
                    auto objPtr = std::move(it->second);
                    workspace.objects_base.erase(it);
                    workspace.objects_base.emplace(newName, std::move(objPtr));
                    break;
                }
                obj.name = newName;
            }
        );        
        main_lua_state.new_usertype<neptune::Color>("Color",
            sol::constructors<Color(Uint8, Uint8, Uint8, Uint8)>(),
            "getR", &Color::getR, "setR", &Color::setR,
            "getG", &Color::getG, "setG", &Color::setG,
            "getB", &Color::getB, "setB", &Color::setB,
            "getA", &Color::getA, "setA", &Color::setA,
            "setFromTable", &Color::setFromTable
        );        
        main_lua_state.new_usertype<neptune::Vector2>("Vector2",
            sol::constructors<Vector2(float, float)>(),
            "getX", &Vector2::getX, "setX", &Vector2::setX,
            "getY", &Vector2::getY, "setY", &Vector2::setY,
            "setFromTable", &Vector2::setFromTable
        );
        main_lua_state.new_usertype<neptune::EventListener>("EventListener",
            sol::constructors<EventListener()>(),
            "AddListener", [](neptune::EventListener& event, sol::function func) {
                event.AddListener(func);
            },
            "Fire", [](neptune::EventListener& event, sol::variadic_args args) {
                event.Fire(args);
            }
        );
        main_lua_state.new_usertype<neptune::Sprite>("Sprite",
            sol::constructors<neptune::Sprite(std::string, int, int, int, int)>(),
            "setColor", [](neptune::Sprite& self, neptune::Color color) {
                self.setColor(color.toSDL());
            },
            "SetMouseCallBack", [](neptune::Sprite& self, sol::function func) {
                self.SetMouseCallBack(func);
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );
        main_lua_state.new_usertype<neptune::Box>("Box",
            sol::constructors<neptune::Box(int, int, int, int, SDL_Color)>(),
            "setColor", [](neptune::Box& self, neptune::Color color) {
                self.setColor(color.toSDL());
            },
            "SetMouseCallBack", [](neptune::Box& self, sol::function func) {
                self.SetMouseCallBack(func);
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );

        main_lua_state.new_usertype<neptune::Triangle>("Triangle",
            sol::constructors<neptune::Triangle(int, int, int, int, SDL_Color)>(),
            "setColor", [](neptune::Triangle& self, neptune::Color color) {
                self.setColor(color.toSDL());
            },
            "SetMouseCallBack", [](neptune::Triangle& self, sol::function func) {
                self.SetMouseCallBack(func);
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );

        main_lua_state.new_usertype<neptune::Circle>("Circle",
            sol::constructors<neptune::Circle(int, int, int, SDL_Color)>(),
            "setColor", [](neptune::Circle& self, neptune::Color color) {
                self.setColor(color.toSDL());
            },
            "SetMouseCallBack", [](neptune::Circle& self, sol::function func) {
                self.SetMouseCallBack(func);
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );
        main_lua_state.new_usertype<Workspace>("Workspace",
            "getDrawObject", [this](Workspace& ws, const std::string& name) -> sol::object {
                auto objVariant = ws.getDrawObject(name);
                if (objVariant) {
                    return std::visit([this](auto& obj) {
                        return sol::make_object(main_lua_state, obj.get());
                    }, *objVariant);
                }
                return sol::lua_nil;
            },
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