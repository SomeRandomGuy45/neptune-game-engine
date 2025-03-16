#include "objects.h"

namespace neptune {

void Box::render(SDL_Renderer *renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    /**
     * Luckily for us, SDL isn't really that difficult to render stuff!
     * I like SDL, its pretty cool!
    **/
    int renderX = x + (SCREEN_WIDTH / 2);
    int renderY = y + (SCREEN_HEIGHT / 2);
    if (renderer == nullptr) {
        game_log("Renderer is null!!! This is not cool!", neptune::CRITICAL);
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {renderX, renderY, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

} // namespace neptune