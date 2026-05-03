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

#ifdef NEPTUNE_FULLSCREEN
#define SCREEN_VALUE SDL_WINDOW_FULLSCREEN
#else
#define SCREEN_VALUE 0
#endif

#ifdef NEPTUNE_RESIZEABLE
#define SCREEN_VALUE_2 SDL_WINDOW_RESIZABLE
#else
#define SCREEN_VALUE_2 0
#endif

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

    std::map<std::string, unsigned char> keycodes = {
        {"NUL", 0},
        {"SOH", 1},
        {"STX", 2},
        {"ETX", 3},
        {"EOT", 4},
        {"ENQ", 5},
        {"ACK", 6},
        {"BEL", 7},
        {"BS", 8},
        {"HT", 9},
        {"LF", 10},
        {"VT", 11},
        {"FF", 12},
        {"CR", 13},
        {"SO", 14},
        {"SI", 15},
        {"DLE", 16},
        {"DC1", 17},
        {"DC2", 18},
        {"DC3", 19},
        {"DC4", 20},
        {"NAK", 21},
        {"SYN", 22},
        {"ETB", 23},
        {"CAN", 24},
        {"EM", 25},
        {"SUB", 26},
        {"ESC", 27},
        {"FS", 28},
        {"GS", 29},
        {"RS", 30},
        {"US", 31},
        {"SPACE", 32},
        {"EXCLAMATION_MARK", 33},
        {"DOUBLE_QUOTE", 34},
        {"HASH", 35},
        {"DOLLAR", 36},
        {"PERCENT", 37},
        {"AMPERSAND", 38},
        {"SINGLE_QUOTE", 39},
        {"LEFT_PAREN", 40},
        {"RIGHT_PAREN", 41},
        {"ASTERISK", 42},
        {"PLUS", 43},
        {"COMMA", 44},
        {"HYPHEN", 45},
        {"PERIOD", 46},
        {"SLASH", 47},
        {"DIGIT_0", 48},
        {"DIGIT_1", 49},
        {"DIGIT_2", 50},
        {"DIGIT_3", 51},
        {"DIGIT_4", 52},
        {"DIGIT_5", 53},
        {"DIGIT_6", 54},
        {"DIGIT_7", 55},
        {"DIGIT_8", 56},
        {"DIGIT_9", 57},
        {"COLON", 58},
        {"SEMICOLON", 59},
        {"LESS_THAN", 60},
        {"EQUAL", 61},
        {"GREATER_THAN", 62},
        {"QUESTION_MARK", 63},
        {"AT", 64},
        {"A", 65},
        {"B", 66},
        {"C", 67},
        {"D", 68},
        {"E", 69},
        {"F", 70},
        {"G", 71},
        {"H", 72},
        {"I", 73},
        {"J", 74},
        {"K", 75},
        {"L", 76},
        {"M", 77},
        {"N", 78},
        {"O", 79},
        {"P", 80},
        {"Q", 81},
        {"R", 82},
        {"S", 83},
        {"T", 84},
        {"U", 85},
        {"V", 86},
        {"W", 87},
        {"X", 88},
        {"Y", 89},
        {"Z", 90},
        {"LEFT_BRACKET", 91},
        {"BACKSLASH", 92},
        {"RIGHT_BRACKET", 93},
        {"CARET", 94},
        {"UNDERSCORE", 95},
        {"GRAVE_ACCENT", 96},
        {"a", 97},
        {"b", 98},
        {"c", 99},
        {"d", 100},
        {"e", 101},
        {"f", 102},
        {"g", 103},
        {"h", 104},
        {"i", 105},
        {"j", 106},
        {"k", 107},
        {"l", 108},
        {"m", 109},
        {"n", 110},
        {"o", 111},
        {"p", 112},
        {"q", 113},
        {"r", 114},
        {"s", 115},
        {"t", 116},
        {"u", 117},
        {"v", 118},
        {"w", 119},
        {"x", 120},
        {"y", 121},
        {"z", 122},
        {"LEFT_BRACE", 123},
        {"VERTICAL_BAR", 124},
        {"RIGHT_BRACE", 125},
        {"TILDE", 126},
        {"DEL", 127}
    };

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

    class Game {
    public:
        ~Game();

        Workspace workspace;
        InputService inputService;
        void init();
        void initLua();
        void addLuaScript(const std::string& scriptPath) {
            luaScripts.push_back(std::filesystem::path(getExecutablePath()).parent_path().string() + "/assets/scripts/" + scriptPath);
        }
        void loadLua(std::shared_mutex& lua_mutex);
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
        // SDL_WINDOW_FULLSCREEN and SDL_WINDOW_RESIZABLE are the only ones we can use
        Uint32 flags = SCREEN_VALUE | SCREEN_VALUE_2;
    private:
        void render(ImGuiIO& io);
        void moveFile(const std::string& outputDirType, const std::string& folderName, const std::string& folderPath, const std::string& execDir);
        
        bool showDemoWin = (USE_DEBUG_DEMO_WIN && GLOBAL_DEBUG);
        bool isDebug = GLOBAL_DEBUG;
        
        SDL_Window* window;
        SDL_Renderer* renderer;
        sol::state main_lua_state;
        std::vector<sol::function> updateFuncs;
        std::vector<std::string> luaScripts;
        Camera camera;
        SceneLoadingService sceneLoadingService;
        PlatformService platformService;
        LinkerService linkerService;
    };

} // namespace neptune