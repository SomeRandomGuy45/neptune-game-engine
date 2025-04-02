#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <thread>

#include "sol/sol.hpp"
#include "objects.h"
#include "helper.h"

namespace neptune {

    using ObjectVariant = std::variant<std::unique_ptr<neptune::Sprite>, 
                                      std::unique_ptr<neptune::Box>, 
                                      std::unique_ptr<neptune::Triangle>, 
                                      std::unique_ptr<neptune::Circle>>;

    class Workspace {
    public:
        std::unordered_map<std::string, ObjectVariant> objects;

        void addObject(std::unique_ptr<neptune::Object> obj, sol::state& lua) {
            std::string objName = obj->name;
            if (auto sprite = dynamic_cast<neptune::Sprite*>(obj.get())) {
                objects[objName] = std::make_unique<neptune::Sprite>(std::move(*sprite));
            } else if (auto box = dynamic_cast<neptune::Box*>(obj.get())) {
                objects[objName] = std::make_unique<neptune::Box>(std::move(*box));
            } else if (auto triangle = dynamic_cast<neptune::Triangle*>(obj.get())) {
                objects[objName] = std::make_unique<neptune::Triangle>(std::move(*triangle));
            } else if (auto circle = dynamic_cast<neptune::Circle*>(obj.get())) {
                objects[objName] = std::make_unique<neptune::Circle>(std::move(*circle));
            }
        }

        ObjectVariant* getObject(const std::string& name) {
            auto it = objects.find(name);
            if (it != objects.end()) {
                return &it->second;
            }
            return nullptr;
        }
    };

    class Game {
    public:
        Workspace workspace;
        void init(const std::string& winName = "Untitled Game");
        void initLua();
        void addObject(std::unique_ptr<neptune::Object> obj) {
            // Construct the ObjectVariant from the specific object type
            if (auto sprite = dynamic_cast<neptune::Sprite*>(obj.get())) {
                workspace.addObject(std::make_unique<neptune::Sprite>(std::move(*sprite)), main_lua_state);
            } else if (auto box = dynamic_cast<neptune::Box*>(obj.get())) {
                workspace.addObject(std::make_unique<neptune::Box>(std::move(*box)), main_lua_state);
            } else if (auto triangle = dynamic_cast<neptune::Triangle*>(obj.get())) {
                workspace.addObject(std::make_unique<neptune::Triangle>(std::move(*triangle)), main_lua_state);
            } else if (auto circle = dynamic_cast<neptune::Circle*>(obj.get())) {
                workspace.addObject(std::make_unique<neptune::Circle>(std::move(*circle)), main_lua_state);
            }
        }        
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
    };

} // namespace neptune