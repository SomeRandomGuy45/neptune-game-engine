#ifndef OBJECTS
#define OBJECTS
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <list>

#include "helper.h"
#include "sol/sol.hpp"

namespace neptune {

enum NEPTUNE_CALLBACK {
    MOUSE = 0x00000001
};

enum NEPTUNE_MUSIC_STATE {
    PLAYING, STOPPED
};

inline bool NEPTUNE_MUSIC_INIT = false;
inline bool NEPTUNE_FONT_INIT = false;

inline std::unordered_map<std::string, TTF_Font*> fonts;

class Color {
public:
    Uint8 r, g, b, a;
    
    Color(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a) 
        : r(_r), g(_g), b(_b), a(_a) {}

    SDL_Color toSDL() const { return {r, g, b, a}; }

    // Getters & Setters for Lua
    Uint8 getR() const { return r; }
    Uint8 getG() const { return g; }
    Uint8 getB() const { return b; }
    Uint8 getA() const { return a; }

    void setR(Uint8 value) { r = value; }
    void setG(Uint8 value) { g = value; }
    void setB(Uint8 value) { b = value; }
    void setA(Uint8 value) { a = value; }
    void setFromTable(sol::table table) {
        r = static_cast<Uint8>(table.get_or("r", 0));
        g = static_cast<Uint8>(table.get_or("g", 0));
        b = static_cast<Uint8>(table.get_or("b", 0));
        a = static_cast<Uint8>(table.get_or("a", 0));
    }
};

class Vector2 {
public:
    float x, y;
    Vector2(float _x, float _y) : 
        x(_x), y(_y) {}
    void setX(float _x) { x = _x; }
    void setY(float _y) { y = _y; }
    float getX() { return x;}
    float getY() { return y;}
    void setFromTable(sol::table table) {
        x = static_cast<float>(table.get_or("x", 0));
        y = static_cast<float>(table.get_or("y", 0));
    }
};

// Non drawble objects
class BaseObject {
public:
    std::string name;
    virtual ~BaseObject() = default;
    void setName(const std::string& _name) { name = _name; }
};

class Camera : BaseObject {
public:
    Camera() {
        name = "Camera";
    }
    float x = 0.0f;
    float y = 0.0f;

    void move(float dx, float dy) {
        x += dx;
        y += dy;
    }
    void setCamera(float dx, float dy) {
        x = dx;
        y = dy;
    }
};

class EventListener : public BaseObject {
public:
    EventListener() {
        name = "EventListener";
    }
    void AddListener(sol::function func);
    void Fire(sol::variadic_args args);
private:
    std::list<sol::function> listeners;
};

class Audio : public BaseObject {
public:
    Audio(std::string _path) {
        if (!NEPTUNE_MUSIC_INIT) {
            if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
                game_log("SDL_mixer could not initialize! SDL_mixer Error: " + std::string(Mix_GetError()), neptune::ERROR);
            }
            Mix_AllocateChannels(32);
            NEPTUNE_MUSIC_INIT = true;
        }
        chunk.reset(Mix_LoadWAV(_path.c_str()));
        if (!chunk.get()) {
            game_log("Couldn't load music! SDL Error: " + std::string(SDL_GetError()), neptune::ERROR);
        }
        name = "Audio";
    }
    void Play();
    void Stop();
    void Destroy();
    void SetLoop(bool _loop);
private:
    std::unique_ptr<Mix_Chunk> chunk;
    NEPTUNE_MUSIC_STATE music_state = STOPPED;
    int channel = 0;
    int loopAmount = 1;
    bool loop = false;
};

class Script : public BaseObject {
public:
    Script() {
        name = "Script";
    }
    
    void saveToFile(std::string file);
private:
    std::string source;
};

// Drawable objects
class Object {
public:
    std::string name;
    virtual ~Object() = default;
    virtual void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera) = 0;
    void setName(const std::string& _name) { name = _name; }
}; 

class Box : public Object {
public:
    Box(float _x, float _y, float _w, float _h, SDL_Color _color)
        : x(_x), y(_y), w(_w), h(_h), color(_color) {
            name = "Box";
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera) override;
    void setColor(SDL_Color newColor) { color = newColor; }
    /*
    * we can just convert the Vector2 class to floats
    */
    void setPosition(float _x, float _y);
    void SetMouseCallBack(sol::function func);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera);
private:
    std::list<sol::function> mouseCallbacks;
    float x, y, w, h;
    SDL_Color color{0,0,0,0};
};

class Triangle : public Object {
public:
    Triangle(float _x, float _y, float _w, float _h, SDL_Color _color)
        : x(_x), y(_y), w(_w), h(_h), color(_color) {
            name = "Triangle";
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera) override;

    void setPosition(float _x, float _y);
    void setColor(SDL_Color newColor) { color = newColor; }
    void SetMouseCallBack(sol::function func);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera);
private:
    std::list<sol::function> mouseCallbacks;
    float x, y, w, h;
    SDL_Color color{0,0,0,0};
};

class Circle : public Object {
public:
    Circle(int _x, int _y, int _radius, SDL_Color _color)
        : x(_x), y(_y), radius(_radius), color(_color) {
            name = "Circle";
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera) override;

    void setPosition(float _x, float _y);
    void setColor(SDL_Color newColor) { color = newColor; }
    void SetMouseCallBack(sol::function func);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera);
private:
    std::list<sol::function> mouseCallbacks;
    int x, y, radius;
    SDL_Color color{0,0,0,0};
};

class Sprite : public Object {
public:
    Sprite(std::string _filePath, float _x, float _y, float _w, float _h)
        : filePath(_filePath), x(_x), y(_y), w(_w), h(_h) {
            name = "Sprite";
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera) override;

    void setPosition(float _x, float _y);
    void setColor(SDL_Color newColor) { color = newColor; }
    void SetMouseCallBack(sol::function func);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera);
private:
    SDL_Texture* texture = nullptr;
    SDL_Color color{0,0,0,0};
    std::list<sol::function> mouseCallbacks;
    std::string filePath;
    float x, y, w, h;
};

class Text : public Object {
public:
    Text(float _x, float _y, float _w, float _h, std::string _text, std::string _fontName = "FreeSans", SDL_Color _text_color = SDL_Color{.r = 255, .g = 255, .b  = 255, .a = 0}, SDL_Color _background_color = SDL_Color{.r = 0, .g = 0, .b = 0, .a = 0}) : 
        x(_x), y(_y), w(_w), h(_h), text_color(_text_color), background_color(_background_color), text(_text), fontName(_fontName) {
            if (!NEPTUNE_FONT_INIT) {
                NEPTUNE_FONT_INIT = true;
                if (TTF_Init() != 0) {
                    game_log("Couldn't start TTF! Error:" + std::string(TTF_GetError()), neptune::CRITICAL);
                    exit(1);
                }
                /*
                * later on force a use of a assets folder
                * something like assets/fonts, would work
                */
                fonts.insert({"FreeSans", TTF_OpenFont("FreeSans.ttf", 24)});
                for (const auto& [name, font] : fonts) {
                    TTF_SetFontHinting(font, TTF_HINTING_LIGHT_SUBPIXEL);
                }
            }
            name = "Text";
        };
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera) override;

    void setPosition(float _x, float _y);
    void setTextColor(SDL_Color newColor) { text_color = newColor; }
    void setBackgroundColor(SDL_Color newColor) { background_color = newColor; }
    void changeText(std::string newText);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT, Camera camera);
private:
    SDL_Texture* texture = nullptr;
    float x, y, w, h;
    SDL_Color text_color;
    SDL_Color background_color;
    std::string text, fontName;
};

}
#endif