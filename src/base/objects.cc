#include "objects.h"

namespace neptune {

void Box::render(SDL_Renderer *renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    /**
     * Luckily for us, SDL isn't really that difficult to render stuff!
     * I like SDL, its pretty cool!
    **/

    /*
     * https://stackoverflow.com/a/25481777
     */
    int renderX = x + ((SCREEN_WIDTH / 2) - (w / 2));
    int renderY = y + ((SCREEN_HEIGHT / 2) - (h / 2));

    if (renderer == nullptr) {
        game_log("Renderer is null!!! This is not cool!", neptune::CRITICAL);
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {renderX, renderY, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void Triangle::render(SDL_Renderer *renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    // we will need a different approach to this

    /*
     * https://stackoverflow.com/a/25481777
    */
    int renderX = x + ((SCREEN_WIDTH / 2) - (w / 2));
    int renderY = y + ((SCREEN_HEIGHT / 2) - (h / 2));
    SDL_Vertex vertices[] = {
        // cast this cast that
        // why man why
        {{static_cast<float>(renderX), static_cast<float>(renderY)}, color, {1, 1}},
        {{static_cast<float>(renderX + w), static_cast<float>(renderY)}, color, {1, 1}},
        {{static_cast<float>(renderX + w / 2), static_cast<float>(renderY - h)}, color, {1, 1}},
    };
    if (renderer == nullptr) {
        game_log("Renderer is null!!! This is not cool!", neptune::CRITICAL);
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);
}

void Circle::render(SDL_Renderer *renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    float renderX = x + ((SCREEN_WIDTH / 2) - (radius));
    float renderY = y + ((SCREEN_HEIGHT / 2) - (radius));

    int offsetx, offsety, d;
    offsetx = 0;
    offsety = radius;
    d = radius -1;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    while (offsety >= offsetx) {
        // Draw horizontal lines to fill the circle
        SDL_RenderDrawLine(renderer, renderX - offsety, renderY + offsetx,
                           renderX + offsety, renderY + offsetx);
        SDL_RenderDrawLine(renderer, renderX - offsetx, renderY + offsety,
                           renderX + offsetx, renderY + offsety);
        SDL_RenderDrawLine(renderer, renderX - offsetx, renderY - offsety,
                           renderX + offsetx, renderY - offsety);
        SDL_RenderDrawLine(renderer, renderX - offsety, renderY - offsetx,
                           renderX + offsety, renderY - offsetx);

        if (d >= 2 * offsetx) {
            d -= 2 * offsetx + 1;
            offsetx++;
        } else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety--;
        } else {
            d += 2 * (offsety - offsetx - 1);
            offsety--;
            offsetx++;
        }
    }
}

} // namespace neptune