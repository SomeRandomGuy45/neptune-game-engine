#include <game.h>
namespace neptune {

    using float_milliseconds = std::chrono::duration<float, std::milli>;

    enum objectTypes {
        SPRITE,
        BOX,
        TRIANGLE,
        CIRCLE,
        TEXT,
        NONE = -1
    };

    std::shared_mutex lua_mutex;

    std::unordered_map<std::string, objectTypes> strToObjectTypes = {
        {"sprite", SPRITE},
        {"box", BOX},
        {"triangle", TRIANGLE},
        {"circle", CIRCLE},
        {"text", TEXT}
    };

    int InputService::getKeyDown() {
        int currentNum = currentKeyDown;
        currentKeyDown = -1;
        return currentNum;
    }

    void InputService::bindKeybind(int key, sol::protected_function func) {
        game_log("Binding key: " + std::to_string(key));
        keybinds[key] = func;
    }

    void InputService::runKeybindFunc(int key) {
        auto it = keybinds.find(key);
        if (it != keybinds.end()) {
            try {
                it->second();
            } catch (const sol::error& e) {
                game_log("Caught error in keybind function: " + std::string(e.what()), neptune::FAULT);
            } catch (...) {
                game_log("Caught unknown error in keybind function!", neptune::FAULT);
            }
        }
    }

    void* LinkerService::loadLib(const std::string& libName) {
        return LIB_LOAD(libName.c_str());
    }

    int LinkerService::removeLib(void *lib)
    {
        return LIB_UNLOAD(lib);
    }

    void Workspace::addObject(std::unique_ptr<neptune::Object> obj, sol::state& lua) {
        if (!obj) return;
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
        if (!obj) return;
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
    Game::~Game() {
        main_lua_state = sol::state();
    }
    void Game::init() {
        #ifdef _WIN32
            ::SetProcessDPIAware();
        #endif
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            game_log("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()), neptune::CRITICAL);
            exit(1);
        }
        if (TTF_Init() != 0) {
            game_log("Couldn't start TTF! Error:" + std::string(TTF_GetError()), neptune::CRITICAL);
            exit(1);
        }
        std::string execPath = getExecutablePath();
        std::string execDir = std::filesystem::path(execPath).parent_path().string();
        game_log("Looking for fonts in: " + execDir + "/assets/fonts");
        for (auto& dir : std::filesystem::recursive_directory_iterator(execDir + "/assets/fonts")) {
        if (dir.is_regular_file() && dir.path().extension() == ".ttf") {
                std::string fontName = dir.path().filename().string();
                fontName = fontName.substr(0, fontName.length() - 4);
                game_log("Loading file: " + fontName + " Dir: " + dir.path().string());
                fonts.insert({fontName, TTF_OpenFont(dir.path().string().c_str(), 24)});
            }
        }
        if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
            game_log("SDL_mixer could not initialize! SDL_mixer Error: " + std::string(Mix_GetError()), neptune::FAULT);
        }
        Mix_AllocateChannels(32);
        //fonts.insert({"FreeSans", TTF_OpenFont("FreeSans.ttf", 24)});
        for (const auto& [name, font] : fonts) {
            TTF_SetFontHinting(font, TTF_HINTING_LIGHT_SUBPIXEL);
        }
        #ifdef SDL_HINT_IME_SHOW_UI
            SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
        #endif
        window = SDL_CreateWindow(sceneLoadingService.gameName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | flags);
        if(!window)
        {
            game_log("Window could not be created! SDL_Error: " + std::string(SDL_GetError()), neptune::CRITICAL);
            exit(1);
        }
        game_log("Made Window");
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (!renderer)
        {
            game_log("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()), neptune::CRITICAL);
            exit(1);
        }
        game_log("Made renderer");
        game_log("Game ready!");
        /*
        std::thread luaScriptThread([this](){
            main_lua_state.script_file("main.lua");
        });
        luaScriptThread.detach();
        */

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup scaling
        float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        bool quit = false;
        game_log("Loading scripts from 1st scene");
        loadLua(lua_mutex);
        while (!quit) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                ImGui_ImplSDL2_ProcessEvent(&e);
                if (e.type == SDL_QUIT) {
                    quit = true;
                    break;
                }
                if (e.type == SDL_WINDOWEVENT) {
                    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        SCREEN_WIDTH = e.window.data1;
                        SCREEN_HEIGHT = e.window.data2;
                        io.DisplaySize = ImVec2((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
                        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
                    }
                }
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    for (const auto& [name, objVariant] : workspace.objects) {
                        bool isClicked = false;
                        std::visit([this, mouseX, mouseY, &isClicked](auto& objPtr) {
                            if (!objPtr) return;
                                
                            isClicked = objPtr->isClicked(mouseX, mouseY, SCREEN_WIDTH, SCREEN_HEIGHT, camera);
                            if (isClicked) {
                                objPtr->DoEventCallback(MOUSE);
                            }
                        }, objVariant);                                       
                    }
                }
                if (e.type == SDL_KEYDOWN) {
                    inputService.setCurrentKeyDown(static_cast<int>(e.key.keysym.sym));
                    inputService.runKeybindFunc(static_cast<int>(e.key.keysym.sym));
                }
            }
            for (const auto& func : updateFuncs) {
                try {
                    func();
                } catch (const sol::error& e) {
                    game_log("Caught error: " + std::string(e.what()), neptune::FAULT);
                } catch (...) {
                    game_log("Caught unknown error!", neptune::FAULT);
                }
            }
             // Start the Dear ImGui frame
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            if (showDemoWin) {
                ImGui::ShowDemoWindow(&showDemoWin);
            }

            render(io);
        }
        for (const auto& [name, font] : fonts) {
            TTF_CloseFont(font);
        }
        game_log("Got kill event!");
        /*
        * Took too long to fix lol!
        */
        updateFuncs.clear();
        workspace.objects.clear();
        workspace.objects_base.clear();
        luaScripts.clear();
        game_log("Cleared lua!");
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        game_log("Destoryed ImGui");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        game_log("SDL window and renderer is gone... Quitting now...");
        SDL_Quit();
    }
    
    void Game::initLua()
    {
        main_lua_state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os, sol::lib::table);
        main_lua_state.set_function("halt", [](float haltTime){
            std::this_thread::sleep_for(float_milliseconds(haltTime * 1000));
        });
        main_lua_state.set_function("setWinTitle", [this](std::string newTitle) {
            if (window == nullptr) {
                game_log("Window is not ready!", neptune::WARNING);
            }
            SDL_SetWindowTitle(window, newTitle.c_str());
        });
        sol::table enumsTable = main_lua_state.create_table();
        main_lua_state["Enums"] = enumsTable;
        sol::table enumKeycodeTypeTable = main_lua_state.create_table();
        for (const auto& [key, value] : keycodes) {
            enumKeycodeTypeTable[key] = value;
        }
        main_lua_state["Enums"]["Keycodes"] = enumKeycodeTypeTable;
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
            },
            "setZIndex" , &Object::setZIndex,
            "getZIndex", &Object::getZIndex
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
            "setFromTable", &Color::setFromTable, "dumpVal", &Color::toSDL
        ); 
        main_lua_state.new_usertype<neptune::Vector2>("Vector2",
            sol::constructors<Vector2(float, float)>(),
            "getX", &Vector2::getX, "setX", &Vector2::setX,
            "getY", &Vector2::getY, "setY", &Vector2::setY,
            "setFromTable", &Vector2::setFromTable
        );
        main_lua_state.new_usertype<neptune::EventListener>("EventListener",
            sol::constructors<EventListener()>(),
            "addListener", [](neptune::EventListener& event, sol::protected_function func) {
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
            //sol::constructors<neptune::Text(float, float, float, float, std::string)>(),
            sol::meta_function::construct,
                [this](float x, float y, float w, float h, std::string text) {
                    std::cout << "called text\n";
                    auto newText = std::make_unique<neptune::Text>(x, y, w, h, text);
                    std::string name = "newText" + std::to_string(objCreatedCount);
                    newText->setName(name);
                    objCreatedCount += 1;
                    addObject(std::move(newText));
                    auto* variant = workspace.getDrawObject(name);
                    auto& obj = std::get<std::unique_ptr<neptune::Text>>(*variant);
                    return sol::make_object(main_lua_state, obj.get());
                },
            "getX", &Text::getX,
            "getY", &Text::getY,
            "setPosition", [](neptune::Text& self, sol::object maybe_vec) {
                if (maybe_vec.is<neptune::Vector2>()) {
                    neptune::Vector2 vec = maybe_vec.as<neptune::Vector2>();
                    self.setPosition(vec.x, vec.y);
                } else {
                    game_log("Warning: setPosition expected Vector2, got something else", neptune::WARNING);
                }
            },
            "changeText", [](neptune::Text& text, std::string newText){
                text.changeText(newText);
            },
            "setTextColor", [](neptune::Text& self, neptune::Color color) {
                self.setTextColor(color.toSDL());
            },
            "setBackgroundColor", [](neptune::Text& self, neptune::Color color) {
                self.setBackgroundColor(color.toSDL());
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );
        main_lua_state.new_usertype<neptune::Sprite>("Sprite",
            //sol::constructors<neptune::Sprite(std::string, int, int, int, int)>(),
            sol::meta_function::construct,
                [this](std::string filePath, float x, float y, float w, float h) {
                    std::cout << "called sprite\n";
                    auto newSprite = std::make_unique<neptune::Sprite>(filePath, x, y, w, h);
                    std::string name = "newSprite" + std::to_string(objCreatedCount);
                    newSprite->setName(name);
                    objCreatedCount += 1;
                    addObject(std::move(newSprite));
                    auto* variant = workspace.getDrawObject(name);
                    auto& obj = std::get<std::unique_ptr<neptune::Sprite>>(*variant);
                    return sol::make_object(main_lua_state, obj.get());
                },
            "getX", &Sprite::getX,
            "getY", &Sprite::getY,
            "setColor", [](neptune::Sprite& self, neptune::Color color) {
                self.setColor(color.toSDL());
            },
            "setMouseCallBack", [](neptune::Sprite& self, sol::protected_function func) {
                self.SetMouseCallBack(func);
            },
            "setPosition", [](neptune::Sprite& self, sol::object maybe_vec) {
                if (maybe_vec.is<neptune::Vector2>()) {
                    neptune::Vector2 vec = maybe_vec.as<neptune::Vector2>();
                    self.setPosition(vec.x, vec.y);
                } else {
                    game_log("Warning: setPosition expected Vector2, got something else", neptune::WARNING);
                }
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );
        main_lua_state.new_usertype<neptune::Box>("Box",
            sol::meta_function::construct,
                [this](float x, float y, float w, float h, neptune::Color c) {
                    std::cout << "called\n";
                    auto newBox = std::make_unique<neptune::Box>(x, y, w, h, c.toSDL());
                    std::string name = "newBox" + std::to_string(objCreatedCount);
                    newBox->setName(name);
                    objCreatedCount += 1;
                    addObject(std::move(newBox));
                    auto* variant = workspace.getDrawObject(name);
                    auto& obj = std::get<std::unique_ptr<neptune::Box>>(*variant);
                    return sol::make_object(main_lua_state, obj.get());
                },
            "getX", &Box::getX,
            "getY", &Box::getY,
            "setColor", [](neptune::Box& self, neptune::Color color) {
                self.setColor(color.toSDL());
            },
            "setMouseCallBack", [](neptune::Box& self, sol::protected_function func) {
                self.SetMouseCallBack(func);
            },
            "setPosition", [](neptune::Box& self, sol::object maybe_vec) {
                if (maybe_vec.is<neptune::Vector2>()) {
                    neptune::Vector2 vec = maybe_vec.as<neptune::Vector2>();
                    self.setPosition(vec.x, vec.y);
                } else {
                    game_log("Warning: setPosition expected Vector2, got something else", neptune::WARNING);
                }
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );

        main_lua_state.new_usertype<neptune::Triangle>("Triangle",
            //sol::constructors<neptune::Triangle(int, int, int, int, SDL_Color)>(),
            sol::meta_function::construct,
                [this](float x, float y, float w, float h, neptune::Color c) {
                    std::cout << "called triangle\n";
                    auto newTriangle = std::make_unique<neptune::Triangle>(x, y, w, h, c.toSDL());
                    std::string name = "newTriangle" + std::to_string(objCreatedCount);
                    newTriangle->setName(name);
                    objCreatedCount += 1;
                    addObject(std::move(newTriangle));
                    auto* variant = workspace.getDrawObject(name);
                    auto& obj = std::get<std::unique_ptr<neptune::Triangle>>(*variant);
                    return sol::make_object(main_lua_state, obj.get());
                },
            "getX", &Triangle::getX,
            "getY", &Triangle::getY,
            "setColor", [](neptune::Triangle& self, neptune::Color color) {
                self.setColor(color.toSDL());
            },
            "setMouseCallBack", [](neptune::Triangle& self, sol::protected_function func) {
                self.SetMouseCallBack(func);
            },
            "setPosition", [](neptune::Triangle& self, sol::object maybe_vec) {
                if (maybe_vec.is<neptune::Vector2>()) {
                    neptune::Vector2 vec = maybe_vec.as<neptune::Vector2>();
                    self.setPosition(vec.x, vec.y);
                } else {
                    game_log("Warning: setPosition expected Vector2, got something else", neptune::WARNING);
                }
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );

        main_lua_state.new_usertype<neptune::Circle>("Circle",
            //sol::constructors<neptune::Circle(int, int, int, SDL_Color)>(),
            sol::meta_function::construct,
                [this](float x, float y, float r, neptune::Color c) {
                    std::cout << "called circle\n";
                    auto newCircle = std::make_unique<neptune::Circle>(x, y, r, c.toSDL());
                    std::string name = "newCircle" + std::to_string(objCreatedCount);
                    newCircle->setName(name);
                    objCreatedCount += 1;
                    addObject(std::move(newCircle));
                    auto* variant = workspace.getDrawObject(name);
                    auto& obj = std::get<std::unique_ptr<neptune::Circle>>(*variant);
                    return sol::make_object(main_lua_state, obj.get());
                },
            "getX", &Circle::getX,
            "getY", &Circle::getY,
            "setColor", [](neptune::Circle& self, neptune::Color color) {
                self.setColor(color.toSDL());
            },
            "setMouseCallBack", [](neptune::Circle& self, sol::protected_function func) {
                self.SetMouseCallBack(func);
            },
            "setPosition", [](neptune::Circle& self, sol::object maybe_vec) {
                if (maybe_vec.is<neptune::Vector2>()) {
                    neptune::Vector2 vec = maybe_vec.as<neptune::Vector2>();
                    self.setPosition(vec.x, vec.y);
                } else {
                    game_log("Warning: setPosition expected Vector2, got something else", neptune::WARNING);
                }
            },
            sol::base_classes, sol::bases<neptune::Object>()
        );
        main_lua_state.new_usertype<PlatformService>("PlatformService",
            "getExecutableDir", &PlatformService::getExecutableDir,
            "getFileFromPicker", &PlatformService::getFromFilePicker,
            "popUpWindow", &PlatformService::popUp
        );
        main_lua_state.new_usertype<InputService>("InputService",
            "getKeyDown", &InputService::getKeyDown,
            "bindKeybind", &InputService::bindKeybind
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
            "moveCamera", [this](Workspace& ws, float dx, float dy) {
                camera.move(dx, dy);
            },
            "setCamera", [this](Workspace& ws, float dx, float dy) {
                camera.setCamera(dx, dy);
            },
            "getCameraX", [this](Workspace& ws) {
                return camera.getX();
            },
            "getCameraY", [this](Workspace& ws) {
                return camera.getY();
            }
        );
        main_lua_state.new_usertype<Game>("Game",
            "Workspace", sol::property([](Game& g) -> Workspace& {
                return g.workspace;
            }),
            "InputService", sol::property([](Game& g) -> InputService& {
                return g.inputService;
            }),
            "PlatformService", sol::property([](Game& g) -> PlatformService& {
                return g.platformService;
            }),
            "loadNewScene", [this](Game& game, std::string newScene) {
                loadNewScene(newScene);
                game_log("Loading new scripts");
                loadLua(lua_mutex);
            }
        );
        main_lua_state["game"] = this;
        game_log("Made Lua engine");
    }
    void Game::moveFile(const std::string& outputDirType, const std::string& folderName, const std::string& folderPath, const std::string& execDir) {
        if (!std::filesystem::exists(folderPath + folderName + "/assets/" + outputDirType)) {
            game_log("Warning: No " + outputDirType + " directory found in place! Skipping...", neptune::WARNING);
            return;
        }
        for (auto& dir : std::filesystem::directory_iterator(folderPath + folderName + "/assets/" + outputDirType)) {
            if (!dir.is_regular_file() && !dir.is_directory()) continue;
            std::string path = dir.path().string();
            auto target = execDir + "/assets/" + outputDirType + "/" + dir.path().filename().string();
            if (std::filesystem::exists(target)) {
                std::filesystem::remove_all(target);
            }
            game_log("Moving file from: " + path + " to " + target);
            try {
                std::filesystem::rename(path, target);
            } catch (const std::filesystem::filesystem_error& e) {
                game_log(std::string("Rename failed, trying copy: ") + e.what());

                std::filesystem::copy(path, target,
                    std::filesystem::copy_options::recursive |
                    std::filesystem::copy_options::overwrite_existing);

                std::filesystem::remove_all(path);
            }
        }
    }
    // TODO... remove this function and make a better way to load games
    void Game::loadGame_DEBUG(std::string gamePath, bool fixPath)
    {
        if (!isDebug) {
            game_log("Tried to call function thats only allowed in DEBUG!", neptune::CRITICAL);
            return;
        }
        zip_t* za;
        zip_stat_t fileStat;
        zip_file_t* file;
        zip_error_t error;

        std::string parentPath;
        std::string lastParentPath;
        std::string execDir = std::filesystem::path(getExecutablePath()).parent_path().string();
        std::string folderName = std::filesystem::path(gamePath).stem().string();
        std::string folderPath = execDir + "/assets/projects/"  + folderName + "/";
        if (fixPath) {
            gamePath = execDir + "/assets/projects/" + gamePath;
        }
        game_log("Extracting place to: " + folderPath + " From: " + gamePath);
        zip_stat_init(&fileStat);

        int err;

        std::filesystem::create_directory(folderPath);

        if ((za = zip_open(gamePath.c_str(), 0, &err)) == NULL) {
            zip_error_init_with_code(&error, err);
            game_log("Cannot open place at: " + gamePath + "  Reason: " + std::string(zip_error_strerror(&error)), neptune::FAULT);
            return;
        }
        for (zip_int64_t i = 0; i < zip_get_num_entries(za, ZIP_FL_UNCHANGED); i++) {
            if (zip_stat_index(za, i, ZIP_FL_UNCHANGED, &fileStat) == -1) {
                zip_error_init_with_code(&error, err);
                game_log("Cannot get file state Reason: " + std::string(zip_error_strerror(&error)), neptune::FAULT);
                return;
            }
            std::string fullOutputPath = folderPath + fileStat.name;
            std::filesystem::path outputPath(fullOutputPath);
            game_log("Extracting file: " + fullOutputPath);
            //std::cout << fileStat.name[strlen(fileStat.name) - 1] << " " << strlen(fileStat.name) << "\n";
            if (outputPath.has_parent_path()) {
                std::filesystem::create_directories(outputPath.parent_path());
            }
            if (fileStat.name[strlen(fileStat.name) - 1] == '/') {
                std::filesystem::create_directories(fullOutputPath);
                continue;
            }
            const zip_uint64_t expectedSize = fileStat.size;
            std::vector<char> contents(expectedSize);

            file = zip_fopen_index(za, i, 0);
            if (!file) {
                game_log("Cannot open file: " + std::string(fileStat.name), neptune::FAULT);
                return;
            }

            zip_int64_t bytesRead = zip_fread(file, contents.data(), expectedSize);
            zip_fclose(file);

            if (bytesRead < 0 || static_cast<zip_uint64_t>(bytesRead) != expectedSize) {
                game_log("zip_fread failed: " + std::string(fileStat.name), neptune::FAULT);
                return;
            }

            std::ofstream out(fullOutputPath, std::ios::binary);
            out.write(contents.data(), bytesRead);
        }
        zip_close(za);
        if (!isVaildGame(folderPath + folderName + "/")) {
            game_log("Unable to load place! Reason: Is not vaild game!", neptune::FAULT);
            return;
        }
        std::ifstream mainJson(folderPath + folderName + "/main.json");
        if (!mainJson.is_open()) {
            game_log("Unable to open main.json!", neptune::FAULT);
            return;
        }
        std::ifstream configJson(folderPath + folderName + "/config.json");
        if (!configJson.is_open()) {
            game_log("Unable to open config.json!", neptune::FAULT);
            return;
        }
        sceneLoadingService.infoJson = json::parse(mainJson);
        sceneLoadingService.configJson = json::parse(configJson);
        mainJson.close();
        configJson.close();
        if (sceneLoadingService.infoJson.contains("projectName")) {
            sceneLoadingService.projectName = sceneLoadingService.infoJson["projectName"];
        }
        if (sceneLoadingService.infoJson.contains("gameName")) {
            sceneLoadingService.gameName = sceneLoadingService.infoJson["gameName"];
        }
        if (sceneLoadingService.infoJson.contains("defaultScene")) {
            sceneLoadingService.defaultScene = sceneLoadingService.infoJson["defaultScene"];
        }
        if (!sceneLoadingService.infoJson.contains("scenes") || !sceneLoadingService.infoJson["scenes"].is_array()) {
            
            game_log("Got no scene data or its not a array! Quitting!", neptune::CRITICAL);
            return;
        }
        objCreatedCount = 0;
        moveFile("sprite_data", folderName, folderPath, execDir);
        moveFile("scripts", folderName, folderPath, execDir);
        moveFile("fonts", folderName, folderPath, execDir);
        moveFile("audio", folderName, folderPath, execDir);
        for (const auto& sceneName : sceneLoadingService.infoJson["scenes"]) {
            std::string pathToScene = folderPath + folderName + "/" + sceneName.get<std::string>() + ".scene";
            std::ifstream stream(pathToScene);
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load(stream);
            if (!result) {
                game_log("Parsed with errors! Scene Name: " + pathToScene, neptune::WARNING);
                continue;
            }
            game_log("Loaded scene: " + sceneName.get<std::string>());
            sceneLoadingService.insertScene(sceneName.get<std::string>(), std::move(doc));
        }
    }
    void Game::loadLua(std::shared_mutex& lua_mutex)
    {
        for (const std::string& file : luaScripts) {
            game_log("Loading script: " + file);
            sol::load_result script = main_lua_state.load_file(file);
            if (!script.valid()) {
                continue;
            }
            sol::table module = script();
            sol::protected_function init_func = module["init"];
            sol::protected_function update_func = module["update"];
            auto moduleBlock = module["isModule"];
            if (moduleBlock.valid()) {
                if (typeid(moduleBlock) == typeid(bool) && moduleBlock.get<bool>() == true) {
                    continue;
                }
            }
            if (!init_func.valid()) {
                game_log("init func is not valid or is empty", neptune::FAULT);
                continue;
            }
            std::thread([init_func, &lua_mutex]() {
                try {
                    std::unique_lock<std::shared_mutex> lock(lua_mutex);
                    init_func();
                } catch (const sol::error& e) {
                    game_log("Caught error: " + std::string(e.what()), neptune::FAULT);
                } catch (...) {
                    game_log("Caught unknown error!", neptune::FAULT);
                }
            }).detach();
            game_log("Ran init function for: " + file);
            updateFuncs.emplace_back(update_func);
        }
    }
    void Game::render(ImGuiIO& io)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        int objectsRendered = 0;
        int numOfObjectsNeedToRender = workspace.objects.size();
        std::set<int> zIndexes;
        for (const auto& [name, objVariant] : workspace.objects) {
            std::visit([&zIndexes](auto& objPtr) {
                if (objPtr) {
                    zIndexes.insert(objPtr->getZIndex());
                }
            }, objVariant);
        }
        for (int currentZIndex : zIndexes) {
            for (const auto& [name, objVariant] : workspace.objects) {
                std::visit([this, &objectsRendered, currentZIndex](auto& objPtr) {
                    if (objPtr != nullptr && !objPtr->didRender && objPtr->getZIndex() == currentZIndex) {
                        objPtr->render(renderer, SCREEN_WIDTH, SCREEN_HEIGHT, camera);
                        objPtr->didRender = true;
                        //std::cout << "Rendered object: " << objPtr->name << " at Z-Index: " << currentZIndex << "\n";
                        objectsRendered++;
                    }
                }, objVariant);
                if (objectsRendered >= numOfObjectsNeedToRender) {
                    break;
                }
            }
            if (objectsRendered >= numOfObjectsNeedToRender) {
                break;
            }
        }
        for (const auto& [name, objVariant] : workspace.objects) {
            std::visit([](auto& objPtr) {
                if (objPtr) {
                    objPtr->didRender = false; 
                }
            }, objVariant);
        }
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_RenderPresent(renderer);
    }
    void SceneLoadingService::insertScene(const std::string& name, pugi::xml_document&& doc) {
        sceneData[name] = std::move(doc);
    }
    pugi::xml_document& SceneLoadingService::getScene(const std::string& name) {
        return sceneData[name];
    }
    void Game::loadNewScene(const std::string& newScene)
    {
        std::string workingSceneName = newScene;
        if (workingSceneName.empty()) {
            game_log("Scene name is empty! Falling back to default scene", neptune::WARNING);
            workingSceneName = sceneLoadingService.defaultScene;
        }
        updateFuncs.clear();
        luaScripts.clear();
        workspace.objects.clear();
        workspace.objects_base.clear();
        pugi::xml_document& doc = sceneLoadingService.getScene(workingSceneName);
        for (pugi::xml_node node : doc.child("game").children()) {
            std::string nodeName = node.name();
            game_log("Node Name: " + nodeName);
            if (nodeName == "object") {
                std::string objName = node.attribute("name").as_string();
                std::string objType = node.attribute("type").as_string();
                std::cout << objType << "\n";
                objectTypes objTypeEnum = objectTypes::NONE;
                if (objType == "sprite" || objType == "box" || objType == "triangle" || objType == "circle" || objType == "text") {
                    objTypeEnum = strToObjectTypes[objType];
                }
                if (objTypeEnum != objectTypes::NONE) {
                    std::unique_ptr<neptune::Object> newObj = nullptr;
                    switch (objTypeEnum) {
                        
                    case objectTypes::BOX: {
                        float x = node.attribute("x").as_float();
                        float y = node.attribute("y").as_float();
                        float w = node.attribute("w").as_float();
                        float h = node.attribute("h").as_float();

                        SDL_Color color = {255, 255, 255, 255};
                        if (node.attribute("hexRbg").as_string()) {
                            std::string colorStr = std::string(node.attribute("hexRbg").as_string()).substr(1); // Remove the '#' character
                            unsigned long colorValue = std::stoul(colorStr, nullptr, 16);
                            color.r = (colorValue >> 16) & 0xFF;
                            color.g = (colorValue >> 8) & 0xFF;
                            color.b = colorValue & 0xFF;
                        }
                        if (node.attribute("transparency").as_string()) {
                            std::string transparencyStr = node.attribute("transparency").as_string();
                            float transparencyValue = std::stof(transparencyStr);
                            color.a = static_cast<Uint8>(transparencyValue * 255);
                        }
                        newObj = std::make_unique<neptune::Box>(x, y, w, h, color);
                        if (node.attribute("zIndex").as_string()) {
                            newObj->setZIndex(node.attribute("zIndex").as_int());
                        } else {
                            newObj->setZIndex(0);
                        }
                        break;
                    }

                    case objectTypes::SPRITE: {
                        float x = node.attribute("x").as_float();
                        float y = node.attribute("y").as_float();
                        float w = node.attribute("w").as_float();
                        float h = node.attribute("h").as_float();
                        std::string spritePath = node.attribute("path").as_string();
                        newObj = std::make_unique<neptune::Sprite>(spritePath, x, y, w, h);
                        if (node.attribute("zIndex").as_string()) {
                            newObj->setZIndex(node.attribute("zIndex").as_int());
                        } else {
                            newObj->setZIndex(0);
                        }
                        break;
                    }

                    case objectTypes::TRIANGLE: {
                        float x = node.attribute("x").as_float();
                        float y = node.attribute("y").as_float();
                        float w = node.attribute("w").as_float();
                        float h = node.attribute("h").as_float();

                        SDL_Color color = {255, 255, 255, 255};
                        if (node.attribute("hexRbg").as_string()) {
                            std::string colorStr = std::string(node.attribute("hexRbg").as_string()).substr(1); // Remove the '#' character
                            unsigned long colorValue = std::stoul(colorStr, nullptr, 16);
                            color.r = (colorValue >> 16) & 0xFF;
                            color.g = (colorValue >> 8) & 0xFF;
                            color.b = colorValue & 0xFF;
                        }
                        if (node.attribute("transparency").as_string()) {
                            std::string transparencyStr = node.attribute("transparency").as_string();
                            float transparencyValue = std::stof(transparencyStr);
                            color.a = static_cast<Uint8>(transparencyValue * 255);
                        }
                        newObj = std::make_unique<neptune::Triangle>(x, y, w, h, color);
                        if (node.attribute("zIndex").as_string()) {
                            newObj->setZIndex(node.attribute("zIndex").as_int());
                        } else {
                            newObj->setZIndex(0);
                        }
                        break;
                    }

                    case objectTypes::CIRCLE: {
                        float x = node.attribute("x").as_float();
                        float y = node.attribute("y").as_float();
                        float radius = node.attribute("r").as_float();

                        SDL_Color color = {255, 255, 255, 255};
                        if (node.attribute("hexRbg").as_string()) {
                            std::string colorStr = std::string(node.attribute("hexRbg").as_string()).substr(1); // Remove the '#' character
                            unsigned long colorValue = std::stoul(colorStr, nullptr, 16);
                            color.r = (colorValue >> 16) & 0xFF;
                            color.g = (colorValue >> 8) & 0xFF;
                            color.b = colorValue & 0xFF;
                        }
                        if (node.attribute("transparency").as_string()) {
                            std::string transparencyStr = node.attribute("transparency").as_string();
                            float transparencyValue = std::stof(transparencyStr);
                            color.a = static_cast<Uint8>(transparencyValue * 255);
                        }
                        newObj = std::make_unique<neptune::Circle>(x, y, radius, color);
                        if (node.attribute("zIndex").as_string()) {
                            newObj->setZIndex(node.attribute("zIndex").as_int());
                        } else {
                            newObj->setZIndex(0);
                        }
                        break;
                    }

                    case objectTypes::TEXT: {
                        float x = node.attribute("x").as_float();
                        float y = node.attribute("y").as_float();
                        float w = node.attribute("w").as_float();
                        float h = node.attribute("h").as_float();
                        std::string textStr = node.attribute("textStr").as_string();

                        SDL_Color textColor = {255, 255, 255, 255};
                        if (node.attribute("textHexRbg").as_string()) {
                            std::string colorStr = std::string(node.attribute("textHexRbg").as_string()).substr(1); // Remove the '#' character
                            unsigned long colorValue = std::stoul(colorStr, nullptr, 16);
                            textColor.r = (colorValue >> 16) & 0xFF;
                            textColor.g = (colorValue >> 8) & 0xFF;
                            textColor.b = colorValue & 0xFF;
                        }
                        if (node.attribute("textTransparency").as_string()) {
                            std::string transparencyStr = node.attribute("textTransparency").as_string();
                            float transparencyValue = std::stof(transparencyStr);
                            textColor.a = static_cast<Uint8>(transparencyValue * 255);
                        }

                        SDL_Color backgroundColor = {0, 0, 0, 0};
                        if (node.attribute("hexRbg").as_string()) {
                            std::string colorStr = std::string(node.attribute("hexRbg").as_string()).substr(1); // Remove the '#' character
                            unsigned long colorValue = std::stoul(colorStr, nullptr, 16);
                            backgroundColor.r = (colorValue >> 16) & 0xFF;
                            backgroundColor.g = (colorValue >> 8) & 0xFF;
                            backgroundColor.b = colorValue & 0xFF;
                        }
                        if (node.attribute("transparency").as_string()) {
                            std::string transparencyStr = node.attribute("transparency").as_string();
                            float transparencyValue = std::stof(transparencyStr);
                            backgroundColor.a = static_cast<Uint8>(transparencyValue * 255);
                        }

                        newObj = std::make_unique<neptune::Text>(x, y, w, h, textStr, "FreeSans", textColor, backgroundColor);
                        if (node.attribute("zIndex").as_string()) {
                            newObj->setZIndex(node.attribute("zIndex").as_int());
                        } else {                            
                            newObj->setZIndex(0);
                        }
                        break;
                    }
                        
                    default: {
                        break;
                    }        
                    }
                    newObj->name = objName;
                    workspace.addObject(std::move(newObj), main_lua_state);
                }
            } else if (nodeName == "script") {
                std::string scriptPath = node.attribute("path").as_string();
                std::cout << "Script path: " << scriptPath << "\n";
                addLuaScript(scriptPath);
            } else if (nodeName == "listener") {
                std::unique_ptr<neptune::EventListener> newListener = std::make_unique<neptune::EventListener>();
                newListener->name = node.attribute("name").as_string();
                std::cout << "Listener name: " << newListener->name << "\n";
                workspace.addBaseObject(std::move(newListener), main_lua_state);
            } else if (nodeName == "audio") {
                std::string audioPath = node.attribute("path").as_string();
                std::unique_ptr<neptune::Audio> newAudio = std::make_unique<neptune::Audio>(audioPath);
                newAudio->name = node.attribute("name").as_string();
                std::cout << "Audio path: " << audioPath << "\n";
                if (node.attribute("isLooping").as_string()) {
                    std::string isLoopingStr = node.attribute("isLooping").as_string();
                    bool isLoopingValue = (isLoopingStr == "true" || isLoopingStr == "1");
                    newAudio->SetLoop(isLoopingValue);
                }
                workspace.addBaseObject(std::move(newAudio), main_lua_state);
            }
        }
    }
} // namespace neptune