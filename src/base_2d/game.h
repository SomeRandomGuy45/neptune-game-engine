#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <utility>
#include <zip.h>


#include "json.hpp"
#include "pugixml.hpp"
#include "sol/sol.hpp"
#include "objects.h"
#include "helper.h"

namespace neptune {

    using ObjectVariant = std::variant<std::unique_ptr<neptune::Sprite>, 
                                      std::unique_ptr<neptune::Box>, 
                                      std::unique_ptr<neptune::Triangle>, 
                                      std::unique_ptr<neptune::Circle>,
                                      std::unique_ptr<neptune::Text>>;
    using BaseObjectVariant = std::variant<std::unique_ptr<neptune::EventListener>,
                                           std::unique_ptr<neptune::Audio>>;

    class Workspace {
    public:
        std::unordered_multimap<std::string, ObjectVariant> objects;
        std::unordered_multimap<std::string, BaseObjectVariant> objects_base;

        void addObject(std::unique_ptr<neptune::Object> obj, sol::state& lua) {
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

        void addBaseObject(std::unique_ptr<neptune::BaseObject> obj, sol::state& lua) {
            std::string objName = obj->name;
            if (auto event = dynamic_cast<neptune::EventListener*>(obj.get())) {
                objects_base.emplace(objName, std::make_unique<neptune::EventListener>(std::move(*event)));
            } else if (auto audio = dynamic_cast<neptune::Audio*>(obj.get())) {
                objects_base.emplace(objName, std::make_unique<neptune::Audio>(std::move(*audio)));
            }
        }

        ObjectVariant* getDrawObject(const std::string& name) {
            auto it = objects.find(name);
            if (it != objects.end()) {
                return &it->second;
            }
            return nullptr;
        }
        BaseObjectVariant* getObject(const std::string& name) {
            auto it = objects_base.find(name);
            if (it!= objects_base.end()) {
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
    };

} // namespace neptune