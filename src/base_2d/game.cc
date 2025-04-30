#include "game.h"

namespace neptune {
    void InputService::addToList(unsigned char keyInput, sol::function func)
    {
        keyList[keyInput].push_back(func);
    }

    std::list<sol::function> InputService::returnListFromKey(unsigned char key)
    {
        if (keyList.count(key) != 0) {
            return keyList[key];
        }
        return std::list<sol::function>();
    }

    void Workspace::addObject(std::unique_ptr<neptune::Object> obj, sol::state& lua) {
        std::string objName = obj->name;
        if (auto sprite = dynamic_cast<neptune::Sprite*>(obj.get())) {
            objects.emplace(objName, std::make_unique<neptune::Sprite>(std::move(*sprite)));
        } else if (auto box = dynamic_cast<neptune::Box*>(obj.get())) {
            objects.emplace(objName, std::make_unique<neptune::Box>(std::move(*box)));
        } else if (auto triangle = dynamic_cast<neptune::Triangle*>(obj.get())) {
            objects.emplace(objName, std::make_unique<neptune::Triangle>(std::move(*triangle)));
        } else if (auto circle = dynamic_cast<neptune::Circle*>(obj.get())) {
            objects.emplace(objName,std::make_unique<neptune::Circle>(std::move(*circle)));
        } else if (auto text = dynamic_cast<neptune::Text*>(obj.get())) {
            objects.emplace(objName,std::make_unique<neptune::Text>(std::move(*text)));
        }
    }
    void Workspace::addBaseObject(std::unique_ptr<neptune::BaseObject> obj, sol::state& lua) {
        std::string objName = obj->name;
        if (auto event = dynamic_cast<neptune::EventListener*>(obj.get())) {
            objects_base.emplace(objName, std::make_unique<neptune::EventListener>(std::move(*event)));
        } else if (auto audio = dynamic_cast<neptune::Audio*>(obj.get())) {
            objects_base.emplace(objName, std::make_unique<neptune::Audio>(std::move(*audio)));
        }
    }
    ObjectVariant* Workspace::getDrawObject(const std::string& name) {
        auto it = objects.find(name);
        if (it != objects.end()) {
            return &it->second;
        }
        return nullptr;
    }
    BaseObjectVariant* Workspace::getObject(const std::string& name) {
        auto it = objects_base.find(name);
        if (it!= objects_base.end()) {
            return &it->second;
        }
        return nullptr;
    }

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
                            isClicked = obj->isClicked(mouseX, mouseY, SCREEN_WIDTH, SCREEN_HEIGHT, camera);
                        }, objVariant);
                        if (isClicked) {
                            std::visit([](auto& obj) {
                                obj->DoEventCallback(MOUSE);
                            }, objVariant);
                        }
                    }
                }
                if (e.type == SDL_KEYDOWN) {
                    for (const auto& callback : inputService.returnListFromKey(e.key.keysym.sym)) { 
                        callback();
                    }
                }
            }
            render();
        }
        if (luaScriptThread.joinable()) {
            luaScriptThread.join();
        }
        for (const auto& [name, font] : fonts) {
            TTF_CloseFont(font);
        }
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    void Game::initLua()
    {
        main_lua_state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os, sol::lib::table);
        main_lua_state.set_function("halt", [](float haltTime){
            /*
            * Multiply by 1000, since SDL_Delay uses ms to delay
            */
            SDL_Delay(haltTime * 1000);
        });
        sol::table enums = main_lua_state.create_named_table("Enums");
        enums.new_enum("Keycodes",
            "NUL", Keycodes::NUL,
            "SOH", Keycodes::SOH,
            "STX", Keycodes::STX,
            "ETX", Keycodes::ETX,
            "EOT", Keycodes::EOT,
            "ENQ", Keycodes::ENQ,
            "ACK", Keycodes::ACK,
            "BEL", Keycodes::BEL,
            "BS", Keycodes::BS,
            "HT", Keycodes::HT,
            "LF", Keycodes::LF,
            "VT", Keycodes::VT,
            "FF", Keycodes::FF,
            "CR", Keycodes::CR,
            "SO", Keycodes::SO,
            "SI", Keycodes::SI,
            "DLE", Keycodes::DLE,
            "DC1", Keycodes::DC1,
            "DC2", Keycodes::DC2,
            "DC3", Keycodes::DC3,
            "DC4", Keycodes::DC4,
            "NAK", Keycodes::NAK,
            "SYN", Keycodes::SYN,
            "ETB", Keycodes::ETB,
            "CAN", Keycodes::CAN,
            "EM", Keycodes::EM,
            "SUB", Keycodes::SUB,
            "ESC", Keycodes::ESC,
            "FS", Keycodes::FS,
            "GS", Keycodes::GS,
            "RS", Keycodes::RS,
            "US", Keycodes::US,
            "SPACE", Keycodes::SPACE,
            "EXCLAMATION_MARK", Keycodes::EXCLAMATION_MARK,
            "DOUBLE_QUOTE", Keycodes::DOUBLE_QUOTE,
            "HASH", Keycodes::HASH,
            "DOLLAR", Keycodes::DOLLAR,
            "PERCENT", Keycodes::PERCENT,
            "AMPERSAND", Keycodes::AMPERSAND,
            "SINGLE_QUOTE", Keycodes::SINGLE_QUOTE,
            "LEFT_PAREN", Keycodes::LEFT_PAREN,
            "RIGHT_PAREN", Keycodes::RIGHT_PAREN,
            "ASTERISK", Keycodes::ASTERISK,
            "PLUS", Keycodes::PLUS,
            "COMMA", Keycodes::COMMA,
            "HYPHEN", Keycodes::HYPHEN,
            "PERIOD", Keycodes::PERIOD,
            "SLASH", Keycodes::SLASH,
            "DIGIT_0", Keycodes::DIGIT_0,
            "DIGIT_1", Keycodes::DIGIT_1,
            "DIGIT_2", Keycodes::DIGIT_2,
            "DIGIT_3", Keycodes::DIGIT_3,
            "DIGIT_4", Keycodes::DIGIT_4,
            "DIGIT_5", Keycodes::DIGIT_5,
            "DIGIT_6", Keycodes::DIGIT_6,
            "DIGIT_7", Keycodes::DIGIT_7,
            "DIGIT_8", Keycodes::DIGIT_8,
            "DIGIT_9", Keycodes::DIGIT_9,
            "COLON", Keycodes::COLON,
            "SEMICOLON", Keycodes::SEMICOLON,
            "LESS_THAN", Keycodes::LESS_THAN,
            "EQUAL", Keycodes::EQUAL,
            "GREATER_THAN", Keycodes::GREATER_THAN,
            "QUESTION_MARK", Keycodes::QUESTION_MARK,
            "AT", Keycodes::AT,
            "LEFT_BRACKET", Keycodes::LEFT_BRACKET,
            "BACKSLASH", Keycodes::BACKSLASH,
            "RIGHT_BRACKET", Keycodes::RIGHT_BRACKET,
            "CARET", Keycodes::CARET,
            "UNDERSCORE", Keycodes::UNDERSCORE,
            "GRAVE_ACCENT", Keycodes::GRAVE_ACCENT,
            "a", Keycodes::a,
            "b", Keycodes::b,
            "c", Keycodes::c,
            "d", Keycodes::d,
            "e", Keycodes::e,
            "f", Keycodes::f,
            "g", Keycodes::g,
            "h", Keycodes::h,
            "i", Keycodes::i,
            "j", Keycodes::j,
            "k", Keycodes::k,
            "l", Keycodes::l,
            "m", Keycodes::m,
            "n", Keycodes::n,
            "o", Keycodes::o,
            "p", Keycodes::p,
            "q", Keycodes::q,
            "r", Keycodes::r,
            "s", Keycodes::s,
            "t", Keycodes::t,
            "u", Keycodes::u,
            "v", Keycodes::v,
            "w", Keycodes::w,
            "x", Keycodes::x,
            "y", Keycodes::y,
            "z", Keycodes::z,
            "LEFT_BRACE", Keycodes::LEFT_BRACE,
            "VERTICAL_BAR", Keycodes::VERTICAL_BAR,
            "RIGHT_BRACE", Keycodes::RIGHT_BRACE,
            "TILDE", Keycodes::TILDE,
            "DEL", Keycodes::DEL
        );
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
        main_lua_state.new_usertype<neptune::Audio>("Audio",
            sol::constructors<Audio(std::string)>(),
            "Play", &Audio::Play,
            "Stop", &Audio::Stop
        );
        main_lua_state.new_usertype<neptune::Text>("Text",
            sol::constructors<neptune::Text(float, float, float, float, std::string)>(),
            "changeText", [](neptune::Text& text, std::string newText){
                text.changeText(newText);
            },
            "setTextColor", [](neptune::Text& self, neptune::Color color) {
                self.setTextColor(color.toSDL());
            },
            "setBackgroundColor", [](neptune::Text& self, neptune::Color color) {
                self.setBackgroundColor(color.toSDL());
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
        main_lua_state.new_usertype<InputService>("InputService",
            "addKeybind", &InputService::addToList
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
            },
            "MoveCamera", [this](Workspace& ws, float dx, float dy) {
                camera.move(dx, dy);
            },
            "SetCamera", [this](Workspace& ws, float dx, float dy) {
                camera.setCamera(dx, dy);
            }
        );
        main_lua_state.new_usertype<Game>("Game",
            "Workspace", sol::property([](Game& g) -> Workspace& {
                return g.workspace;
            }),
            "InputService", sol::property([](Game& g) -> InputService& {
                return g.inputService;
            })
        );
        main_lua_state["game"] = this;
        game_log("Made Lua engine");
    }
    void Game::loadGame(std::string gamePath)
    {
        zip_t* za;
        zip_stat_t fileStat;
        zip_file_t* file;
        zip_error_t error;

        std::string parentPath;
        std::string lastParentPath;
        std::string execDir = std::filesystem::path(getExecutablePath()).parent_path().string();
        std::string folderPath = execDir + "/" + gamePath.substr(0, gamePath.length() - 4) + "/";

        zip_stat_init(&fileStat);

        int err;

        std::filesystem::create_directory(folderPath);

        if ((za = zip_open(gamePath.c_str(), 0, &err)) == NULL) {
            zip_error_init_with_code(&error, err);
            game_log("Cannot open place at: " + gamePath + "  Reason: " + std::string(zip_error_strerror(&error)), neptune::ERROR);
            return;
        }
        for (zip_int64_t i = 0; i < zip_get_num_entries(za, ZIP_FL_UNCHANGED); i++) {
            if (zip_stat_index(za, i, ZIP_FL_UNCHANGED, &fileStat) == -1) {
                zip_error_init_with_code(&error, err);
                game_log("Cannot get file state Reason: " + std::string(zip_error_strerror(&error)), neptune::ERROR);
                return;
            }
            std::string fullOutputPath = folderPath + fileStat.name;
            std::filesystem::path outputPath(fullOutputPath);
            game_log("Extracting file: " + fullOutputPath);
            std::cout << fileStat.name[strlen(fileStat.name) - 1] << " " << strlen(fileStat.name) << "\n";
            if (outputPath.has_parent_path()) {
                std::filesystem::create_directories(outputPath.parent_path());
            }
            if (fileStat.name[strlen(fileStat.name) - 1] == '/') {
                std::filesystem::create_directories(fullOutputPath);
                continue;
            }
            char* contents = new char[fileStat.size];
            if ((file = zip_fopen_index(za, i, ZIP_FL_UNCHANGED)) == NULL) {
                zip_error_init_with_code(&error, err);
                game_log("Cannot get file Reason: " + std::string(zip_error_strerror(&error)), neptune::ERROR);
                return;
            }
            zip_fread(file, contents, fileStat.size);
            if (!std::ofstream(folderPath + std::string(fileStat.name)).write(contents, fileStat.size)) {
                game_log("Cannot write file! with path: " + std::string(fileStat.name));
                return;
            }
        }
        zip_close(za);
    }
    void Game::render()
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        for (const auto& [name, objVariant] : workspace.objects) {
            std::visit([this](auto& obj) {
                obj->render(renderer, SCREEN_WIDTH, SCREEN_HEIGHT, camera);
            }, objVariant);
        }
        SDL_RenderPresent(renderer);
    }
} // namespace neptune