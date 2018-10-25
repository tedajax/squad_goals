#include <vector>
#include <random>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "algebra.h"

struct agent {
    vec2 position = vec2::ZERO;
    f32 rotation = 0;
    f32 speed = 0;

    vec2 target = vec2::ZERO;
};

const f32 dt = 1.f / 60.f;

int main(int argc, char* argv[]) {
    std::vector<agent> agents;

    std::random_device rd;
    std::default_random_engine randGen(rd());
    std::uniform_real_distribution<f32> distX(0.f, 1280.f);
    std::uniform_real_distribution<f32> distY(0.f, 720.f);

    for (int i = 0; i < 9; ++i) {
        agent ag;
        ag.position.x = distX(randGen);
        ag.position.y = distY(randGen);
        ag.target.x = distX(randGen);
        ag.target.y = distY(randGen);
        agents.push_back(ag);
    }


    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Squad Goals", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* dudeSurface = IMG_Load("assets/dude.png");
    SDL_Texture* dudeTexture = SDL_CreateTextureFromSurface(renderer, dudeSurface);

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

        // UPDATE
        {
            int mx, my;
            SDL_GetMouseState(&mx, &my);

            for (auto& agent : agents) {
                /*agent.target.x = (f32)mx;
                agent.target.y = (f32)my;*/
                
                vec2 targetDelta = agent.target - agent.position;

                if (targetDelta.len2() < 4) {
                    agent.target.x = distX(randGen);
                    agent.target.y = distY(randGen);
                }

                vec2 targetDirection = vec2::normalize(targetDelta);
                f32 targetAngle = math::angle_from_vec2(targetDirection);
                
                agent.rotation = math::lerp_angle(agent.rotation, targetAngle, 0.05f * dt);
                vec2 agentDirection = math::vec2_from_angle(agent.rotation);
                
                agent.speed = math::lerp(5, 0.1f, math::clamp01(math::delta_angle(agent.rotation, targetAngle) / 20.f));

                agent.position += agentDirection * agent.speed * (1.f / 60);
            }
        }

        // RENDER
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            for (const auto& agent : agents) {
                SDL_Rect rect = {
                    (int)agent.position.x - 16, (int)agent.position.y - 16,
                    32, 32
                };
                SDL_RenderCopyEx(renderer, dudeTexture, nullptr, &rect, agent.rotation, nullptr, SDL_FLIP_NONE);
                SDL_RenderDrawPoint(renderer, agent.target.x, agent.target.y);
            }

            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
