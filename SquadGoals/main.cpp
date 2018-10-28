#include <vector>
#include <random>
#include <iostream>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <effolkronium/random.hpp>

#include "algebra.h"
#include "perlin.h"
#include "flowfield.h"

struct agent {
    vec2 position = vec2::ZERO;
    vec2 velocity = vec2::ZERO;
    vec2 acceleration = vec2::ZERO;
    f32 rotation = 0;
    
    vec2 target = vec2::ZERO;

    vec2 future;
    f32 wanderAngle;
    f32 wanderTimer = 0.f;

    f32 maxSpeed = 200.f;
    f32 maxAccel = 150.f;
    f32 projectionDist = 100.f;
    f32 projectionRadius = 35.f;
    f32 wanderMult = 10.f;
    f32 wanderInterval = 1 / 60.f;
};

using Random = effolkronium::random_static;

void render_circle(SDL_Renderer* renderer, int cx, int cy, int radius);
void render_direction(SDL_Renderer* renderer, vec2 center, vec2 direction, f32 length);

int main(int argc, char* argv[]) {
    Random::seed(1);

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    TTF_Font* debugFont = TTF_OpenFont("assets/prstartk.ttf", 16);

    SDL_Window* window = SDL_CreateWindow("Squad Goals", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* dudeSurface = IMG_Load("assets/dude.png");
    SDL_Texture* dudeTexture = SDL_CreateTextureFromSurface(renderer, dudeSurface);

    std::vector<agent> agents;
    flow_field flowField(1280.f, 720.f, 32.f);

    perlin_gen perlin(6);
    flowField.perlin_angles(perlin, 0.f);

    for (int i = 0; i < 25; ++i) {
        agent ag;
        ag.position.x = (f32)Random::get(0, 1280);
        ag.position.y = (f32)Random::get(0, 720);
        ag.target.x = (f32)Random::get(0, 1280);
        ag.target.y = (f32)Random::get(0, 720);
        ag.wanderAngle = Random::get(0.f, 360.f);
        agents.push_back(ag);
    }

    int fps = 0;
    u64 ticks = SDL_GetPerformanceCounter();
    u64 lastTicks = ticks;
    u64 frameTicks = 0;
    int framesInSecond = 0;
    f32 dt = 0.f;

    f32 a = 0.f;

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

        // TIME
        {
            ticks = SDL_GetPerformanceCounter();
            u64 frequency = SDL_GetPerformanceFrequency();

            u64 diff = ticks - lastTicks;
            diff *= (1000000000 / frequency);

            lastTicks = ticks;

            dt = (f32)diff / 1000000000.f;

            ++framesInSecond;
            frameTicks += diff;
            if (frameTicks >= 1000000000) {
                frameTicks -= 1000000000;
                fps = framesInSecond;
                framesInSecond = 0;
                std::cout << "FPS: " << fps << std::endl;
            }
        }

        // UPDATE
        {
            a += 1.f * dt;
            flowField.perlin_angles(perlin, math::ping_pong(a, 100.f));

            int mx, my;
            SDL_GetMouseState(&mx, &my);

            for (auto& agent : agents) {
                // WANDER
                [&agent, dt] {

                    vec2 velocity = agent.velocity;
                    if (agent.velocity == vec2::ZERO) {
                        velocity = math::vec2_from_angle(Random::get(0.f, 360.f));
                    }

                    agent.future = agent.position + vec2::normalize(velocity) * agent.projectionDist;

                    agent.wanderTimer -= dt;
                    if (agent.wanderTimer <= 0) {
                        agent.wanderAngle += Random::get<f32>(-agent.wanderMult, agent.wanderMult);
                        agent.wanderTimer += agent.wanderInterval;
                    }

                    agent.target.x = agent.future.x + math::cos(agent.wanderAngle) * agent.projectionRadius;
                    agent.target.y = agent.future.y + math::sin(agent.wanderAngle) * agent.projectionRadius;
                }();

                // SEEK
                [&agent, &flowField, dt] {
                    vec2 targetDir;
                    f32 targetDist;

                    vec2 targetDelta = agent.target - agent.position;

                    targetDelta.decompose(targetDir, targetDist);

                    vec2 desired;
                    const f32 BORDER_SIZE = 128.f;

                    if (agent.position.x < BORDER_SIZE) {
                        desired = vec2(agent.maxSpeed, agent.velocity.y);
                    }
                    else if (agent.position.x > 1280 - BORDER_SIZE) {
                        desired = vec2(-agent.maxSpeed, agent.velocity.y);
                    }
                    else if (agent.position.y < BORDER_SIZE) {
                        desired = vec2(agent.velocity.x, agent.maxSpeed);
                    }
                    else if (agent.position.y > 720 - BORDER_SIZE) {
                        desired = vec2(agent.velocity.x, -agent.maxSpeed);
                    }
                    else {
                        vec2 wander = targetDir * math::clamp01(targetDist / 100) * agent.maxSpeed;
                        vec2 flow = flowField.get(agent.position.x, agent.position.y) * agent.maxSpeed;
                        desired = (wander + flow) / 2.f;
                        //desired = flow;
                    }

                    vec2 steer = desired - agent.velocity;
                    steer.limit(agent.maxAccel);

                    agent.acceleration = steer;
                }();

                // TICK MOVEMENT
                [&agent, dt] {
                    agent.velocity += agent.acceleration * dt;
                    
                    agent.rotation = math::angle_from_vec2(vec2::normalize(agent.velocity));

                    agent.position += agent.velocity * dt;
                }();
            }
        }

        // RENDER
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            for (int x = 0; x < flowField.width(); ++x) {
                for (int y = 0; y < flowField.height(); ++y) {
                    vec2 v = flowField.get(x, y);
                    render_direction(renderer, flowField.cell_center(x, y), v, flowField.cell_size());
                }
            }

            for (const auto& agent : agents) {
                SDL_Rect rect = {
                    (int)agent.position.x - 16, (int)agent.position.y - 16,
                    32, 32
                };

                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                SDL_RenderDrawLine(renderer, (int)agent.position.x, (int)agent.position.y, (int)agent.future.x, (int)agent.future.y);
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawLine(renderer, (int)agent.future.x, (int)agent.future.y, (int)agent.target.x, (int)agent.target.y);
                render_circle(renderer, (int)agent.future.x, (int)agent.future.y, (int)agent.projectionRadius);

                SDL_RenderCopyEx(renderer, dudeTexture, nullptr, &rect, agent.rotation, nullptr, SDL_FLIP_NONE);
            }

            // render FPS
            if (false)
            {
                char buffer[16] = { 0 };
                snprintf(buffer, 16, "FPS: %d", fps);
                SDL_Color white = { 255, 255, 255, 255 };
                SDL_Surface* fpsTextSurface = TTF_RenderText_Solid(debugFont, buffer, white);
                SDL_Texture* fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsTextSurface);

                SDL_Rect destRect = { 0, 0, 100, 20 };
                SDL_RenderCopy(renderer, fpsTexture, nullptr, &destRect);

                SDL_DestroyTexture(fpsTexture);
                SDL_FreeSurface(fpsTextSurface);
            }

            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}

// Midpoint Circle Algorithm
void render_circle(SDL_Renderer* renderer, int cx, int cy, int radius) {
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1); // radius * 2

    while (x >= y) {
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);

        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void render_direction(SDL_Renderer* renderer, vec2 center, vec2 direction, f32 length)
{
    vec2 p0 = center - direction * (length / 2);
    vec2 p1 = center + direction * (length / 2);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(renderer, (int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y);
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderDrawPoint(renderer, (int)p1.x, (int)p1.y);
}