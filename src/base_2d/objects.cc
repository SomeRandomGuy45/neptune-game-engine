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

void Box::SetMouseCallBack(sol::function func)
{
    mouseCallbacks.push_back(func);
}

void Box::DoEventCallback(NEPTUNE_CALLBACK callback)
{
    if (callback == MOUSE && !mouseCallbacks.empty()) {
        for (const auto& callback : mouseCallbacks) {
            callback();
        }
    }
    else {
        game_log("Invalid callback type", neptune::WARNING);
    }
}

bool Box::isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT) {
    int renderX = x + ((SCREEN_WIDTH / 2) - (w / 2));
    int renderY = y + ((SCREEN_HEIGHT / 2) - (h / 2));
    return (mouseX >= renderX && mouseX <= renderX + w && mouseY >= renderY && mouseY <= renderY + h);
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

void Triangle::SetMouseCallBack(sol::function func)
{
    mouseCallbacks.push_back(func);
}

void Triangle::DoEventCallback(NEPTUNE_CALLBACK callback)
{
    if (callback == MOUSE && !mouseCallbacks.empty()) {
        for (const auto& callback : mouseCallbacks) {
            callback();
        }
    }
    else {
        game_log("Invalid callback type", neptune::WARNING);
    }
}

bool Triangle::isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT) {
    // Calculate triangle's screen coordinates
    int x1 = x + ((SCREEN_WIDTH / 2) - (w / 2));
    int y1 = y + ((SCREEN_HEIGHT / 2) - (h / 2));
    
    int x2 = x1 + w;
    int y2 = y1;
    
    int x3 = x1 + (w / 2);
    int y3 = y1 - h;

    // Function to calculate area of a triangle given three points
    auto area = [](int x1, int y1, int x2, int y2, int x3, int y3) {
        return abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
    };

    // Compute areas
    double A = area(x1, y1, x2, y2, x3, y3);  // Area of main triangle
    double A1 = area(mouseX, mouseY, x2, y2, x3, y3); // Sub-triangle 1
    double A2 = area(x1, y1, mouseX, mouseY, x3, y3); // Sub-triangle 2
    double A3 = area(x1, y1, x2, y2, mouseX, mouseY); // Sub-triangle 3

    // Check if sum of sub-triangle areas equals original triangle's area
    return (A == (A1 + A2 + A3));
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

void Circle::SetMouseCallBack(sol::function func)
{
    mouseCallbacks.push_back(func);
}

void Circle::DoEventCallback(NEPTUNE_CALLBACK callback)
{
    if (callback == MOUSE && !mouseCallbacks.empty()) {
        for (const auto& callback : mouseCallbacks) {
            callback();
        }
    }
    else {
        game_log("Invalid callback type", neptune::WARNING);
    }
}

bool Circle::isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT) {
    float renderX = x + ((SCREEN_WIDTH / 2) - (radius));
    float renderY = y + ((SCREEN_HEIGHT / 2) - (radius));
    float dx = mouseX - renderX;
    float dy = mouseY - renderY;
    return (dx * dx + dy * dy) <= (radius * radius);
}

void Sprite::render(SDL_Renderer *renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    if (texture == nullptr) {
        SDL_Surface* loadedSurface = IMG_Load(filePath.c_str());
        if (!loadedSurface) {
            game_log("Couldn't as surface!" + std::string(SDL_GetError()), neptune::ERROR);
            return;
        }

        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);

        if (!texture) {
            game_log("Couldn't load texture!" + std::string(SDL_GetError()), neptune::ERROR);
        }
    }
    
    int renderX = x + ((SCREEN_WIDTH / 2) - (w / 2));
    int renderY = y + ((SCREEN_HEIGHT / 2) - (h / 2));
    SDL_Rect renderRect = {renderX, renderY, w, h};
    if (color.a != 0 || color.r != 0 || color.g != 0 || color.b != 0) {
        SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
        SDL_SetTextureAlphaMod(texture, color.a);
    }
    SDL_RenderCopy(renderer, texture, nullptr, &renderRect);
}

void Sprite::SetMouseCallBack(sol::function func)
{
    mouseCallbacks.push_back(func);
}

void Sprite::DoEventCallback(NEPTUNE_CALLBACK callback)
{
    if (callback == MOUSE && !mouseCallbacks.empty()) {
        for (const auto& callback : mouseCallbacks) {
            callback();
        }
    }
    else {
        game_log("Invalid callback type", neptune::WARNING);
    }
}

bool Sprite::isClicked(int mouseX, int mouseY, int SCREEN_WIDTH, int SCREEN_HEIGHT) {
    int renderX = x + ((SCREEN_WIDTH / 2) - (w / 2));
    int renderY = y + ((SCREEN_HEIGHT / 2) - (h / 2));
    return (mouseX >= renderX && mouseX <= renderX + w && mouseY >= renderY && mouseY <= renderY + h);
}

void EventListener::AddListener(sol::function func)
{
    listeners.push_back(func);
}

void EventListener::Fire(sol::variadic_args args)
{
    for (const auto& listener : listeners) {
        listener(args);
    }
}

void Audio::Play() {
    if (music_state != STOPPED) {
        game_log("Audio is playing!", neptune::WARNING);
        return;
    }
    if (!chunk) {
        game_log("No audio loaded!", neptune::WARNING);
        return;
    }
    if (channel == -2) {
        game_log("Audio is gone!", neptune::ERROR);
        return;
    }
    int num = loopAmount;
    num = loop ? 0 : 1; // -1 to play indefinitely, 1 to play once
    channel = Mix_PlayChannel(-1, chunk.get(), num - 1);
    if (channel == -1) {
        game_log("Error playing audio! " + std::string(Mix_GetError()), neptune::ERROR);
        return;
    }
    music_state = PLAYING;
}

void Audio::Stop() {
    if (music_state != PLAYING) {
        game_log("Audio is stopped!", neptune::WARNING);
        return;
    }
    if (!chunk) {
        game_log("No audio loaded!", neptune::WARNING);
        return;
    }
    if (channel == -2) {
        game_log("Audio is gone!", neptune::ERROR);
        return;
    }
    Mix_Pause(channel);
    music_state = STOPPED;
}

void Audio::SetLoop(bool _loop) {
    loop = _loop;
}

void Audio::Destroy() {
    if (chunk) {
        Mix_FreeChunk(chunk.release());
        chunk = nullptr;
    }
    music_state = STOPPED;
    channel = -2;
}

} // namespace neptune