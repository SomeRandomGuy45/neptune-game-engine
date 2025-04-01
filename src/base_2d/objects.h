#ifndef OBJECTS
#define OBJECTS
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "helper.h"
#include "sol/sol.hpp"

namespace neptune {

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
};


class Object {
public:
    std::string name;
    virtual ~Object() = default;
    virtual void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) = 0;
    void setName(const std::string& _name) { name = _name; }
    virtual void setColor(SDL_Color newColor) = 0;
};

class Box : public Object {
public:
    Box(int _x, int _y, int _w, int _h, SDL_Color _color)
        : x(_x), y(_y), w(_w), h(_h), color(_color) {
            name = "Box"; // Correct way to set the name
    }
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) override;
    void setColor(SDL_Color newColor) override { color = newColor; }
private:
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
    void setColor(SDL_Color newColor) override { color = newColor; }
private:
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
    void setColor(SDL_Color newColor) override { color = newColor; }
private:
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
    void setColor(SDL_Color newColor) override { color = newColor; }
private:
    SDL_Texture* texture = nullptr;
    SDL_Color color{0,0,0,0};
    std::string filePath;
    int x, y, w, h;
};


}
#endif