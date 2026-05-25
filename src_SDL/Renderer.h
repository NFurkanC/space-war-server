#pragma once
#include <SDL.h>
#include <vector>
#include "Packets.h"
#include "Camera.h"

class Renderer {
public:
    int windowWidth;
    int windowHeight;

    Renderer(const char* title, int width, int height);
    ~Renderer();

    void clear();
    void present();
    void resize(int w, int h);
    void drawScene(const std::vector<std::vector<LinePacket>>& polygons, const Camera& camera);

    SDL_Window* getWindow() const { return window; }
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    void drawSinglePolygon(const std::vector<LinePacket>& lines, const Camera& camera);
};
