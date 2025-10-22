#include <game.h>
namespace neptune {

    using float_milliseconds = std::chrono::duration<float, std::milli>;

    void InputService::addToList(unsigned char keyInput, sol::function func)
    {
        keyList[keyInput].push_back(func);
    }

    void InputService::clearList() {
        keyList.clear();
    }

    std::list<sol::function> InputService::returnListFromKey(unsigned char key)
    {
        if (keyList.count(key) != 0) {
            return keyList[key];
        }
        return std::list<sol::function>();
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
        game_log("Getting scripts from: " + execDir + "/assets/scripts");
        for (auto& dir : std::filesystem::recursive_directory_iterator(execDir + "/assets/scripts")) {
            if (!dir.is_regular_file() || dir.path().extension().string() != ".lua") continue;
            game_log("Loading script: " + dir.path().filename().string());
            sol::load_result script = main_lua_state.load_file(dir.path().string());
            if (!script.valid()) {
                continue;
            }
            sol::table module = script();
            sol::function init_func = module["init"];
            sol::function update_func = module["update"];
            if (!init_func.valid()) {
                game_log("init func is not valid or is empty", neptune::ERROR);
                continue;;
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
            game_log("Ran init function for: " + dir.path().filename().string());
            updateFuncs.emplace_back(update_func);
        }
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
        main_lua_state = sol::state();
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
            "setMouseCallBack", [](neptune::Sprite& self, sol::function func) {
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
            "setMouseCallBack", [](neptune::Box& self, sol::function func) {
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
            "setMouseCallBack", [](neptune::Triangle& self, sol::function func) {
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
            "setMouseCallBack", [](neptune::Circle& self, sol::function func) {
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
            "MoveCamera", [this](Workspace& ws, float dx, float dy) {
                camera.move(dx, dy);
            },
            "SetCamera", [this](Workspace& ws, float dx, float dy) {
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
            char* contents = new char[fileStat.size];
            if ((file = zip_fopen_index(za, i, ZIP_FL_UNCHANGED)) == NULL) {
                zip_error_init_with_code(&error, err);
                game_log("Cannot get file Reason: " + std::string(zip_error_strerror(&error)), neptune::ERROR);
                return;
            }
            zip_fread(file, contents, fileStat.size);
            if (!std::ofstream(folderPath + std::string(fileStat.name)).write(contents, fileStat.size)) {
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
        for (const auto& sceneName : sceneLoadingService.infoJson["scenes"]) {
            std::string pathToScene = folderPath + folderName + "/" + sceneName.get<std::string>() + ".scene";
            pugi::xml_document doc;
            pugi::xml_parse_result result = doc.load_file(pathToScene.c_str());
            if (!result) {
                game_log("Parsed with errors! Scene Name: " + pathToScene, neptune::WARNING);
                continue;
            }
            game_log("Loaded scene: " + sceneName.get<std::string>());
            sceneLoadingService.insertScene(sceneName, std::move(doc));
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
    void SceneLoadingService::loadNewScene(std::string newScene)
    {
        /*
        * TODO
        */
    }
} // namespace neptune