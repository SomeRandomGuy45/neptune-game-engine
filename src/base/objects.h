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
    virtual void render(SDL_Renderer* renderer) = 0;
};

class Box : public Object {
public:
    Box(int x, int y, int w, int h, SDL_Color color) : x(x), y(y), w(w), h(h), color(color) {};
    void render(SDL_Renderer* renderer) override;
private:
    int x, y, w, h;
    SDL_Color color;
};

}
#endif