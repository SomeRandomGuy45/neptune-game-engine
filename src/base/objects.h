#ifndef OBJECTS
#define OBJECTS
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "helper.h"

namespace neptune {

class Object {
public:
    std::string name;
    virtual ~Object() = default;
    virtual void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) = 0;
};

class Box : public Object {
public:
    Box(int _x, int _y, int _w, int _h, SDL_Color _color)
        : x(_x), y(_y), w(_w), h(_h), color(_color) {}
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) override;
private:
    int x, y, w, h;
    SDL_Color color;
};

class Triangle : public Object {
public:
    Triangle(int _x, int _y, int _w, int _h, SDL_Color _color)
        : x(_x), y(_y), w(_w), h(_h), color(_color) {}
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) override;
private:
    int x, y, w, h;
    SDL_Color color;
};

class Circle : public Object {
public:
    Circle(int _x, int _y, int _radius, SDL_Color _color)
        : x(_x), y(_y), radius(_radius), color(_color) {}
    void render(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT) override;
private:
    int x, y, radius;
    SDL_Color color;
};

}
#endif