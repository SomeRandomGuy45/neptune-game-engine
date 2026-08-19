#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <utility>
#include <filesystem>
#include <sstream>
#include <cctype>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <queue>
#include <tuple>
#include <zip.h>

#include "json.hpp"
#include "pugixml.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "objects.h"
#include "gameLoaderHelper.h"
#include "helper.h"
#include "struct.h"
#include "os/platform.h"

#ifdef NEPTUNE_DEBUG_GLOBAL
#define GLOBAL_DEBUG true
#else
#define GLOBAL_DEBUG false
#endif

#ifdef NEPTUNE_DEBUG_IMGUI
#define USE_DEBUG_DEMO_WIN true
#else
#define USE_DEBUG_DEMO_WIN false
#endif

using json = nlohmann::json;

namespace neptune {

    using ObjectVariant = std::variant<std::unique_ptr<neptune::Sprite>, 
                                      std::unique_ptr<neptune::Box>, 
                                      std::unique_ptr<neptune::Triangle>, 
                                      std::unique_ptr<neptune::Circle>,
                                      std::unique_ptr<neptune::Text>>;
    using BaseObjectVariant = std::variant<std::unique_ptr<neptune::EventListener>,
                                           std::unique_ptr<neptune::Audio>>;

    struct colorStruct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    class SceneLoadingService {
    public:
        std::string projectName = "Untitled Project";
        std::string gameName = "Untitled Game";
        std::string defaultScene = "scene1";
        std::string currentScene = "scene1";

        json infoJson;
        json configJson;

        void insertScene(const std::string& name, pugi::xml_document&& doc);
        pugi::xml_document& getScene(const std::string& name);
    private:
        /*
        * To keep it simple, the scene data is stored like this
        * SceneName, XML Doc
        * IDEA: One day or soon, add a scene type?
        */
        std::unordered_map<std::string, pugi::xml_document> sceneData;
    };
    class LinkerService {
    public:
        void loadNewLink(const std::string& libName);
        void addFunctionFromLink(const std::string& libName, const std::string& funcName);
        std::vector<std::string> returnAllFunctionsFromLink(const std::string& libName);
    private:
        void* loadLib(const std::string& libName);
        template <typename Func>
        Func getFunc(void* lib, const std::string& funcName) {
            return reinterpret_cast<Func>(LIB_GETFUNC(lib, funcName.c_str()));
        }
        int removeLib(void* lib);
    };

    class InputService {
    public:
        void bindKeybind(int key, sol::protected_function func);
        void runKeybindFunc(int key);
        void setCurrentKeyDown(int key) { currentKeyDown = key; }
        void clearKey() {keybinds.clear();}
        int getKeyDown();
    private:
        std::map<int, sol::protected_function> keybinds;
        int currentKeyDown = -1;
    };

    class PlatformService {
    public:
        std::string getExecutableDir() {
            return std::filesystem::path(getExecutablePath()).parent_path().string();
        };
        std::string getFromFilePicker() {
            return getFileFromPicker();
        }
        int popUp(const char* title, const char* message) {
            return popUpWindow(title, message);
        }
    };

    class Workspace {
    public:
        std::unordered_multimap<std::string, ObjectVariant> objects;
        std::unordered_multimap<std::string, BaseObjectVariant> objects_base;

        void addObject(std::unique_ptr<neptune::Object> obj, sol::state& lua);
        void addBaseObject(std::unique_ptr<neptune::BaseObject> obj, sol::state& lua);

        ObjectVariant* getDrawObject(const std::string& name);
        BaseObjectVariant* getObject(const std::string& name);
    };

    class ImGuiService {
    public:
        // new object
        void newWindow(const std::string& windowName, bool& passBool);
        void newText(const std::string& text);
        void newBtn(const std::string& btnText, sol::protected_function callbackFunc);
        
        // end object
        void endWindow();

        // helpers
        void sameLine();
    private:
        // return code, return string, will exit?
        std::vector<std::tuple<int, std::string, bool>> returnCalls = {
            {-1, "Internal Error", true},
            {1, "Not running on update func! Expect GUI issues", false}
        };
        bool isRunningOnUpdateFunc = false;
    };

    class Game {
    public:
        Workspace workspace;
        InputService inputService;
        void init();
        void initLua();
        void addLuaScript(const std::string& scriptPath) {
            luaScripts.push_back(std::filesystem::path(getExecutablePath()).parent_path().string() + "/assets/scripts/" + scriptPath);
        }
        void loadLua(std::mutex& lua_mutex);
        void addObject(std::unique_ptr<neptune::Object> obj) {
            workspace.addObject(std::move(obj), main_lua_state);
        }    
        void addBaseObject(std::unique_ptr<neptune::BaseObject> obj) {
            workspace.addBaseObject(std::move(obj), main_lua_state);
        }
        void loadGame_DEBUG(std::string gamePath, bool fixPath);
        void loadNewScene(const std::string& newScene = "");
        int SCREEN_WIDTH = 640;
        int SCREEN_HEIGHT = 480;
        // SDL_WINDOW_FULLSCREEN_DESKTOP and SDL_WINDOW_RESIZABLE are the only ones we can use
        Uint32 flags = 0;
    private:
        void render(ImGuiIO& io);
        void moveFile(const std::string& outputDirType, const std::string& folderName, const std::string& folderPath, const std::string& execDir);
        static void luaError(sol::optional<std::string> maybe_msg);
        static int luaExceptionHandler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description);
        bool showDemoWin = (USE_DEBUG_DEMO_WIN && GLOBAL_DEBUG);
        bool isDebug = GLOBAL_DEBUG;
        bool quit = false;
        bool newSceneLoading = false;
        bool setToMaxRes = false;
        int objCreatedCount = 0;
        Uint64 startTime;
        
        SDL_Window* window;
        SDL_Renderer* renderer;
        sol::state main_lua_state;
        std::string newSceneToLoad;
        std::vector<sol::function> updateFuncs;
        std::vector<std::string> luaScripts;
        std::queue<sol::protected_function> initFuncs;
        mutable std::shared_mutex sceneMutex;
        Camera camera;
        SceneLoadingService sceneLoadingService;
        PlatformService platformService;
        LinkerService linkerService;
        ImGuiService imGuiService;
    };

} // namespace neptune