#include <vector>
#include <ctime>
#include <cstdlib>

#include <SDL2/SDL.h>

#include "algebra.h"

int main(int argc, char* argv[]) {
    std::vector<vec2> points;

    std::srand((unsigned)time(0));

    for (int i = 0; i < 200; ++i) {
        points.push_back(vec2(std::rand() % 1280, std::rand() % 720));
    }

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Squad Goals", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool isRunning = true;
    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.scancode != SDL_SCANCODE_ESCAPE) {
                        break;
                    }
                case SDL_QUIT:
                    isRunning = false;
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (const auto& pt : points) {
            SDL_RenderDrawPoint(renderer, pt.x, pt.y);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
