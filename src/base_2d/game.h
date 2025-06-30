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
#include <zip.h>


#include "json.hpp"
#include "pugixml.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "objects.h"
#include "gameLoaderHelper.h"
#include "helper.h"

using json = nlohmann::json;

namespace neptune {

    using ObjectVariant = std::variant<std::unique_ptr<neptune::Sprite>, 
                                      std::unique_ptr<neptune::Box>, 
                                      std::unique_ptr<neptune::Triangle>, 
                                      std::unique_ptr<neptune::Circle>,
                                      std::unique_ptr<neptune::Text>>;
    using BaseObjectVariant = std::variant<std::unique_ptr<neptune::EventListener>,
                                           std::unique_ptr<neptune::Audio>>;


    enum Keycodes : unsigned char {
        NUL = 0,   // Null
        SOH = 1,   // Start of Header
        STX = 2,   // Start of Text
        ETX = 3,   // End of Text
        EOT = 4,   // End of Transmission
        ENQ = 5,   // Enquiry
        ACK = 6,   // Acknowledge
        BEL = 7,   // Bell
        BS  = 8,   // Backspace
        HT  = 9,   // Horizontal Tab
        LF  = 10,  // Line Feed
        VT  = 11,  // Vertical Tab
        FF  = 12,  // Form Feed
        CR  = 13,  // Carriage Return
        SO  = 14,  // Shift Out
        SI  = 15,  // Shift In
        DLE = 16,  // Data Link Escape
        DC1 = 17,  // Device Control 1
        DC2 = 18,  // Device Control 2
        DC3 = 19,  // Device Control 3
        DC4 = 20,  // Device Control 4
        NAK = 21,  // Negative Acknowledge
        SYN = 22,  // Synchronous Idle
        ETB = 23,  // End of Transmission Block
        CAN = 24,  // Cancel
        EM  = 25,  // End of Medium
        SUB = 26,  // Substitute
        ESC = 27,  // Escape
        FS  = 28,  // File Separator
        GS  = 29,  // Group Separator
        RS  = 30,  // Record Separator
        US  = 31,  // Unit Separator
        SPACE = 32,
        EXCLAMATION_MARK = 33,
        DOUBLE_QUOTE = 34,
        HASH = 35,
        DOLLAR = 36,
        PERCENT = 37,
        AMPERSAND = 38,
        SINGLE_QUOTE = 39,
        LEFT_PAREN = 40,
        RIGHT_PAREN = 41,
        ASTERISK = 42,
        PLUS = 43,
        COMMA = 44,
        HYPHEN = 45,
        PERIOD = 46,
        SLASH = 47,
        DIGIT_0 = 48,
        DIGIT_1 = 49,
        DIGIT_2 = 50,
        DIGIT_3 = 51,
        DIGIT_4 = 52,
        DIGIT_5 = 53,
        DIGIT_6 = 54,
        DIGIT_7 = 55,
        DIGIT_8 = 56,
        DIGIT_9 = 57,
        COLON = 58,
        SEMICOLON = 59,
        LESS_THAN = 60,
        EQUAL = 61,
        GREATER_THAN = 62,
        QUESTION_MARK = 63,
        AT = 64,
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        LEFT_BRACKET = 91,
        BACKSLASH = 92,
        RIGHT_BRACKET = 93,
        CARET = 94,
        UNDERSCORE = 95,
        GRAVE_ACCENT = 96,
        a = 97,
        b = 98,
        c = 99,
        d = 100,
        e = 101,
        f = 102,
        g = 103,
        h = 104,
        i = 105,
        j = 106,
        k = 107,
        l = 108,
        m = 109,
        n = 110,
        o = 111,
        p = 112,
        q = 113,
        r = 114,
        s = 115,
        t = 116,
        u = 117,
        v = 118,
        w = 119,
        x = 120,
        y = 121,
        z = 122,
        LEFT_BRACE = 123,
        VERTICAL_BAR = 124,
        RIGHT_BRACE = 125,
        TILDE = 126,
        DEL = 127
    };

    class GameLoadingService {
    public:
        std::string Name;
        std::string DefaultScene;

        json InfoJson;
        json ConfigJson;

    private:
        /*
        * We are a 1 time service, only used when the game engine starts up!
        * Idea: Maybe also use it for games, and not just the game engine??
        */
        bool HasLoadedGame = false;
    };

    class SceneLoadingService {
    public:
        std::string CurrentScene = "None";
    
        void loadNewScene(std::string newScene);
    private:
        /*
        * To keep it simple, the scene data is stored like this
        * SceneName, XML Doc
        * IDEA: One day or soon, add a scene type?
        */
        std::unordered_map<std::string, pugi::xml_document> SceneData;
    };


    class DL_Service {
    public:
        /*
        * TODO:
        *   Like everything...
        */
        //void* getFunc(void);
    };

    class InputService {
    public:
        void addToList(unsigned char keyInput, sol::function func);
        void clearList();
        std::list<sol::function> returnListFromKey(unsigned char key);
    private:
        std::unordered_map<unsigned char, std::list<sol::function>> keyList;
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
        void init(const std::string& winName = "Untitled Game");
        void initLua();
        void addObject(std::unique_ptr<neptune::Object> obj) {
            workspace.addObject(std::move(obj), main_lua_state);
        }    
        void addBaseObject(std::unique_ptr<neptune::BaseObject> obj) {
            workspace.addBaseObject(std::move(obj), main_lua_state);
        }
        void loadGame(std::string gamePath);
        int SCREEN_WIDTH = 640;
        int SCREEN_HEIGHT = 480;
        // SDL_WINDOW_FULLSCREEN and SDL_WINDOW_RESIZABLE are the only ones we can use
        Uint32 flag_1 = 0;
        Uint32 flag_2 = 0;
    private:
        void render();
        SDL_Window* window;
        SDL_Renderer* renderer;
        sol::state main_lua_state;
        std::vector<sol::function> updateFuncs;
        Camera camera;
        GameLoadingService gameLoadingService;
        SceneLoadingService sceneLoadingService;
    };

} // namespace neptune