#ifndef OBJECTS
#define OBJECTS
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <list>

#include "helper.h"
#include "sol/sol.hpp"

namespace neptune {

enum NEPTUNE_CALLBACK {
    MOUSE = 0x00000001
};

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
    void setX(int _x) { x = _x; }
    void setY(int _y) { y = _y; }
    float getX() { return x;}
    float getY() { return y;}
    void setFromTable(sol::table table) {
        x = static_cast<int>(table.get_or("x", 0));
        y = static_cast<int>(table.get_or("y", 0));
    }
};

class Object {
public:
    std::string name;
    virtual ~Object() = default;
    virtual void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) = 0;
    void setName(const std::string& _name) { name = _name; }
}; 

// Non drawble objects
class BaseObject {
public:
    std::string name;
    virtual ~BaseObject() = default;
    void setName(const std::string& _name) { name = _name; }
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

class Box : public Object {
public:
    Box(int _x, int _y, int _w, int _h, SDL_Color _color)
        : x(_x), y(_y), w(_w), h(_h), color(_color) {
            name = "Box";
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) override;
    void setColor(SDL_Color newColor) { color = newColor; }
    void SetMouseCallBack(sol::function func);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT);
private:
    std::list<sol::function> mouseCallbacks;
    int x, y, w, h;
    SDL_Color color{0,0,0,0};
};

class Triangle : public Object {
public:
    Triangle(int _x, int _y, int _w, int _h, SDL_Color _color)
        : x(_x), y(_y), w(_w), h(_h), color(_color) {
            name = "Triangle";
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) override;
    void setColor(SDL_Color newColor) { color = newColor; }
    void SetMouseCallBack(sol::function func);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT);
private:
    std::list<sol::function> mouseCallbacks;
    int x, y, w, h;
    SDL_Color color{0,0,0,0};
};

class Circle : public Object {
public:
    Circle(int _x, int _y, int _radius, SDL_Color _color)
        : x(_x), y(_y), radius(_radius), color(_color) {
            name = "Circle";
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) override;
    void setColor(SDL_Color newColor) { color = newColor; }
    void SetMouseCallBack(sol::function func);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT);
private:
    std::list<sol::function> mouseCallbacks;
    int x, y, radius;
    SDL_Color color{0,0,0,0};
};

class Sprite : public Object {
public:
    Sprite(std::string _filePath, int _x, int _y, int _w, int _h)
        : filePath(_filePath), x(_x), y(_y), w(_w), h(_h) {
            name = "Sprite";
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) override;
    void setColor(SDL_Color newColor) { color = newColor; }
    void SetMouseCallBack(sol::function func);
    void DoEventCallback(NEPTUNE_CALLBACK callback);

    bool isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT);
private:
    std::list<sol::function> mouseCallbacks;
    SDL_Texture* texture = nullptr;
    SDL_Color color{0,0,0,0};
    std::string filePath;
    int x, y, w, h;
};


}
#endif