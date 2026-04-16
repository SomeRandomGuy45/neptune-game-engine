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

    std::unordered_map<std::string, objectTypes> strToObjectTypes = {
        {"sprite", SPRITE},
        {"box", BOX},
        {"triangle", TRIANGLE},
        {"circle", CIRCLE},
        {"text", TEXT}
    };

    void InputService::addToList(unsigned char keyInput, sol::protected_function func)
    {
        keyList[keyInput].push_back(func);
    }

    void InputService::clearList() {
        keyList.clear();
    }

    std::list<sol::protected_function> InputService::returnListFromKey(unsigned char key)
    {
        if (keyList.count(key) != 0) {
            return keyList[key];
        }
        return std::list<sol::protected_function>();
    }

    void* Linker_Service::loadLib(const std::string& libName) {
        return LIB_LOAD(libName.c_str());
    }

    void* Linker_Service::getFunc(void* lib, const std::string& funcName) {
        return LIB_GETFUNC(lib, funcName.c_str());
    }

    int Linker_Service::removeLib(void *lib)
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


        std::string execPath = getExecutablePath();
        std::string execDir = std::filesystem::path(execPath).parent_path().string();
        bool quit = false;
        std::shared_mutex lua_mutex;
        game_log("Loading scripts");
        loadLua(lua_mutex);
        while (!quit) {
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                ImGui_ImplSDL2_ProcessEvent(&e);
                if (e.type == SDL_QUIT) {
                    quit = true;
                    break;
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
                    for (const auto& callback : inputService.returnListFromKey(e.key.keysym.sym)) { 
                        callback();
                    }
                }
            }
            for (const auto& func : updateFuncs) {
                try {
                    func();
                } catch (const sol::error& e) {
                    game_log("Caught error: " + std::string(e.what()), neptune::ERROR);
                } catch (...) {
                    game_log("Caught unknown error!", neptune::ERROR);
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
        inputService.clearList();
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
        std::cout << "hi\n";
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
            sol::constructors<neptune::Text(float, float, float, float, std::string)>(),
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
            sol::constructors<neptune::Sprite(std::string, int, int, int, int)>(),
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
            sol::constructors<neptune::Box(int, int, int, int, SDL_Color)>(),
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
            sol::constructors<neptune::Triangle(int, int, int, int, SDL_Color)>(),
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
            sol::constructors<neptune::Circle(int, int, int, SDL_Color)>(),
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
            })
        );
        main_lua_state["game"] = this;
        game_log("Made Lua engine");
    }
    // TODO... remove this function and make a better way to load games
    void Game::loadGame_DEBUG(std::string gamePath)
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
        std::string folderName = gamePath.substr(0, gamePath.length() - (std::filesystem::path(gamePath).extension().string().length()));
        std::string folderPath = execDir + "/assets/projects/"  + folderName + "/";
        gamePath = execDir + "/assets/projects/" + gamePath;
        game_log("Extracting place to: " + folderPath + " From: " + gamePath);
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
            //std::cout << fileStat.name[strlen(fileStat.name) - 1] << " " << strlen(fileStat.name) << "\n";
            if (outputPath.has_parent_path()) {
                std::filesystem::create_directories(outputPath.parent_path());
            }
            if (fileStat.name[strlen(fileStat.name) - 1] == '/') {
                std::filesystem::create_directories(fullOutputPath);
                continue;
            }
            std::vector<char> contents(fileStat.size);
            if ((file = zip_fopen_index(za, i, ZIP_FL_UNCHANGED)) == NULL) {
                zip_error_init_with_code(&error, err);
                game_log("Cannot get file Reason: " + std::string(zip_error_strerror(&error)), neptune::ERROR);
                return;
            }
            zip_fread(file, contents.data(), fileStat.size);
            if (!std::ofstream(folderPath + std::string(fileStat.name)).write(contents.data(), fileStat.size)) {
                game_log("Cannot write file! with path: " + std::string(fileStat.name), neptune::ERROR);
                return;
            }
        }
        zip_close(za);
        if (!isVaildGame(folderPath + folderName + "/")) {
            game_log("Unable to load place! Reason: Is not vaild game!", neptune::ERROR);
            return;
        }
        std::ifstream mainJson(folderPath + folderName + "/main.json");
        if (!mainJson.is_open()) {
            game_log("Unable to open main.json!", neptune::ERROR);
            return;
        }
        std::ifstream configJson(folderPath + folderName + "/config.json");
        if (!configJson.is_open()) {
            game_log("Unable to open config.json!", neptune::ERROR);
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
        for (auto& dir : std::filesystem::directory_iterator(folderPath + folderName + "/assets/sprite_data")) {
            if (!dir.is_regular_file()) continue;
            std::string path = dir.path().string();
            game_log("Moving sprite data from: " + path + " to " + execDir + "/assets/sprite_data/" + dir.path().filename().string());
            std::filesystem::rename(path, execDir + "/assets/sprite_data/" + dir.path().filename().string());
        }
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
            if (!init_func.valid()) {
                game_log("init func is not valid or is empty", neptune::ERROR);
                continue;
            }
            std::thread([init_func, &lua_mutex]() {
                try {
                    std::unique_lock<std::shared_mutex> lock(lua_mutex);
                    init_func();
                } catch (const sol::error& e) {
                    game_log("Caught error: " + std::string(e.what()), neptune::ERROR);
                } catch (...) {
                    game_log("Caught unknown error!", neptune::ERROR);
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
    pugi::xml_document& SceneLoadingService::getAllScenes(const std::string& name) {
        return sceneData[name];
    }
    void Game::loadNewScene(std::string newScene)
    {
        pugi::xml_document& doc = sceneLoadingService.getAllScenes(newScene);
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
                        int x = node.attribute("x").as_int();
                        int y = node.attribute("y").as_int();
                        int w = node.attribute("w").as_int();
                        int h = node.attribute("h").as_int();

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
                        int x = node.attribute("x").as_int();
                        int y = node.attribute("y").as_int();
                        int w = node.attribute("w").as_int();
                        int h = node.attribute("h").as_int();
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
                        int x = node.attribute("x").as_int();
                        int y = node.attribute("y").as_int();
                        int w = node.attribute("w").as_int();
                        int h = node.attribute("h").as_int();

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
                        int x = node.attribute("x").as_int();
                        int y = node.attribute("y").as_int();
                        int radius = node.attribute("r").as_int();

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
                        int x = node.attribute("x").as_int();
                        int y = node.attribute("y").as_int();
                        int w = node.attribute("w").as_int();
                        int h = node.attribute("h").as_int();
                        std::string textStr = node.attribute("text").as_string();

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

                        newObj = std::make_unique<neptune::Text>(x, y, w, h, textStr);
                        static_cast<neptune::Text*>(newObj.get())->setTextColor(textColor);
                        static_cast<neptune::Text*>(newObj.get())->setBackgroundColor(backgroundColor);
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
            }
        }
    }
} // namespace neptune