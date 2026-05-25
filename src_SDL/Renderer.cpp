#include "Renderer.h"
#include <stdexcept>

Renderer::Renderer(const char* title, int width, int height) : windowWidth(width), windowHeight(height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("Failed to init SDL");
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (!window) throw std::runtime_error("Failed to create window");

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) throw std::runtime_error("Failed to create renderer");
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Renderer::resize(int w, int h) {
    windowWidth = w;
    windowHeight = h;
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

void Renderer::drawScene(const std::vector<std::vector<LinePacket>>& polygons, const Camera& camera) {
    clear();
    for (const auto& poly : polygons) {
        drawSinglePolygon(poly, camera);
    }
    present();
}

void Renderer::drawSinglePolygon(const std::vector<LinePacket>& lines, const Camera& camera) {
    if (lines.empty()) return;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (const auto& line : lines) {
        ScreenPos p1 = camera.worldToScreen(line.a.x, line.a.y);
        ScreenPos p2 = camera.worldToScreen(line.b.x, line.b.y);
        SDL_RenderDrawLine(renderer, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y);
    }
}
