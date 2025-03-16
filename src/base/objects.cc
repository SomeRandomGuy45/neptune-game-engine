#include "objects.h"

void neptune::Box::render(SDL_Renderer *renderer)
{
    /**
     * Luckily for us, SDL isn't really that difficult to render stuff....
     * I like SDL, its pretty cool!
    **/
    if (renderer == nullptr) {
        game_log("Renderer is null!!! This is not cool!", neptune::CRITICAL);
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}