#include <vector>
#include <random>
#include <iostream>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <GL/glew.h>

#include <Box2D/Box2D.h>

#include <effolkronium/random.hpp>

#include "algebra.h"
#include "perlin.h"
#include "flowfield.h"
#include "box2dSdlDebugDraw.h"
#include "path.h"
#include "input_state.h"
#include "renderer.h"

struct agent {
    vec2 position = vec2::ZERO;
    vec2 velocity = vec2::ZERO;
    vec2 acceleration = vec2::ZERO;
    f32 rotation = 0;

    vec2 target = vec2::ZERO;

    vec2 future;
    f32 wanderAngle;
    f32 wanderTimer = 0.f;

    f32 maxSpeed = 10.f;
    f32 maxAccel = 5.f;
    f32 projectionDist = 4.f;
    f32 projectionRadius = 0.1f;
    f32 wanderMult = 10.f;
    f32 wanderInterval = 1 / 60.f;

    b2Body* body;
};

static void init_agent(agent& ag, b2World& world) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0f, 4.0f);
    ag.body = world.CreateBody(&bodyDef);


    b2CircleShape shapeDef;
    shapeDef.m_radius = 0.25f;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shapeDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;

    ag.body->CreateFixture(&fixtureDef);
    ag.body->SetTransform(ag.position, ag.rotation);
}

using Random = effolkronium::random_static;

void render_circle(SDL_Renderer* renderer, int cx, int cy, int radius);
void render_direction(SDL_Renderer* renderer, vec2 center, vec2 direction, f32 length);
void world_to_screen(f32 wx, f32 wy, int& sx, int& sy);
void screen_to_world(int sx, int sy, f32& wx, f32& wy);

int main(int argc, char* argv[]) {
    Random::seed(1);
    input_state input;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    TTF_Font* debugFont = TTF_OpenFont("assets/prstartk.ttf", 16);



    SDL_Window* window = SDL_CreateWindow("Squad Goals", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    
    renderer r;
    r.init(window);

    camera cam;
    cam.position = glm::vec3(1.f, 0.f, 10.f);

    path agentPath(path_dir::kCW, 2.5f, { vec2(-10, -8), vec2(0, -7), vec2(10, -8), vec2(14, 0), vec2(10, 8), vec2(0, 7), vec2(-10, 8), vec2(-14, 0) });

    b2World world(vec2::ZERO);

    //box2dSdlDebugDraw sdlDebugDraw(renderer);
    //world.SetDebugDraw(&sdlDebugDraw);

    const i32 velocityIterations = 8;
    const i32 positionIterations = 8;

    /*SDL_Surface* dudeSurface = IMG_Load("assets/dude.png");
    SDL_Texture* dudeTexture = SDL_CreateTextureFromSurface(renderer, dudeSurface);*/

    std::vector<agent> agents;
    flow_field flowField(40.f, 36.f, 1.f, -20.f, -18.f);

    perlin_gen perlin(10000);
    flowField.perlin_angles(perlin, 1.f);

    const int AGENT_COUNT = 1;

    for (int i = 0; i < AGENT_COUNT; ++i) {
        agent ag;
        ag.position.x = (f32)Random::get(-20, 20);
        ag.position.y = (f32)Random::get(-11, 11);
        ag.target.x = (f32)Random::get(-20, 20);
        ag.target.y = (f32)Random::get(-11, 11);
        ag.wanderAngle = Random::get(0.f, 360.f);
        init_agent(ag, world);
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

        input.update();

        int mx, my;
        SDL_GetMouseState(&mx, &my);

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
            // camera movement
            {
                const f32 speed = 5.f * dt;
                if (input.get_key(SDL_SCANCODE_D)) {
                    cam.move_relative(glm::vec3(speed, 0, 0));
                }

                if (input.get_key(SDL_SCANCODE_A)) {
                    cam.move_relative(glm::vec3(-speed, 0, 0));
                }

                if (input.get_key(SDL_SCANCODE_W)) {
                    cam.move_relative(glm::vec3(0, 0, -speed));
                }

                if (input.get_key(SDL_SCANCODE_S)) {
                    cam.move_relative(glm::vec3(0, 0, speed));
                }
            }

            {
                auto agent = agents[0];
                vec2 pos = agent.body->GetPosition();
                f32 rot = agent.body->GetAngle();

                if (input.get_key(SDL_SCANCODE_LEFT)) {
                    pos.x -= 10.f * dt;
                }

                if (input.get_key(SDL_SCANCODE_RIGHT)) {
                    pos.x += 10.f * dt;
                }

                if (input.get_key(SDL_SCANCODE_UP)) {
                    pos.y -= 10.f * dt;
                }

                if (input.get_key(SDL_SCANCODE_DOWN)) {
                    pos.y += 10.f * dt;
                }

                agents[0].body->SetTransform(pos, rot);
            }

            a += 0.1f * dt;
            flowField.perlin_angles(perlin, 1.f + math::ping_pong(a, 3.f), math::ping_pong(a, 100.f));


            for (auto& agent : agents) {
                agent.position = agent.body->GetPosition();
                agent.velocity = agent.body->GetLinearVelocity();

                // TARGET
                [&agent, &agentPath, dt] {
                    vec2 velocity = agent.velocity;

                    vec2 predicted = agent.position + vec2::normalize(agent.velocity) * 2.f;

                    vec2 pathDir;
                    vec2 nearest = agentPath.nearest(predicted, pathDir);
                    f32 pathDist = vec2::dist(nearest, agent.position);
                    if (pathDist > agentPath.path_width()) {
                        agent.target = nearest + pathDir * 1.f;
                        agent.future = predicted;
                    }
                    else {
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
                    }
                }();

                // SEPARATE
                vec2 separation = [&agent, &agents]() -> vec2 {
                    vec2 sum = vec2::ZERO;
                    int count = 0;
                    for (auto& other : agents) {
                        if (&other == &agent) {
                            continue;
                        }

                        vec2 delta = agent.position - other.position;
                        f32 d = delta.len();
                        if (d > 0 && d < 2.f) {
                            sum += vec2::normalize(delta) / d;
                            ++count;
                        }
                    }

                    if (count > 0) {
                        sum /= count;
                        sum.normalize();
                        sum *= agent.maxSpeed;
                        return sum;
                    }
                    else {
                        return vec2::ZERO;
                    }
                }();

                // SEEK
                [&agent, &flowField, &agentPath, &separation, dt] {
                    vec2 targetDir;
                    f32 targetDist;

                    vec2 targetDelta = agent.target - agent.position;

                    targetDelta.decompose(targetDir, targetDist);

                    vec2 desired;
                    const f32 BORDER_SIZE = 128.f;

                    //if (agent.position.x < -20) {
                    //    desired = vec2(agent.maxSpeed, agent.velocity.y);
                    //}
                    //else if (agent.position.x > 20) {
                    //    desired = vec2(-agent.maxSpeed, agent.velocity.y);
                    //}
                    //else if (agent.position.y < -12) {
                    //    desired = vec2(agent.velocity.x, agent.maxSpeed);
                    //}
                    //else if (agent.position.y > 12) {
                    //    desired = vec2(agent.velocity.x, -agent.maxSpeed);
                    //}
                    //else {
                    //}

                    //vec2 wander = targetDir * math::clamp01(targetDist / 100) * agent.maxSpeed;
                    vec2 flow = flowField.get(agent.position);
                    //desired = flow + separation;
                    //desired = flow;
                    //desired = wander + separation;

                    vec2 movement = targetDir * math::clamp01(targetDist / 40);

                    desired = flow + movement;

                    desired = 1.f * movement + 0.01f * flow + 0.01f * separation;
                    desired.normalize();
                    desired *= agent.maxSpeed;

                    vec2 steer = desired - agent.velocity;
                    steer.limit(agent.maxAccel);

                    agent.body->ApplyForce(steer, vec2::ZERO, true);
                }();

                // TICK MOVEMENT
                [&agent, dt] {
                    agent.rotation = math::angle_from_vec2(vec2::normalize(agent.velocity));
                    agent.body->SetTransform(agent.position, agent.rotation);
                }();
            }

            world.Step(dt, velocityIterations, positionIterations);
        }

        // RENDER
        {
            for (f32 a = 0; a < 360; a += 10) {
                f32 b = a + 10;
                vec2 p0 = math::vec2_from_angle(a);
                vec2 p1 = math::vec2_from_angle(b);
                r.line(glm::vec3(p0.x, p0.y, 0), glm::vec3(p1.x, p1.y, 0), glm::vec4(1, 1, 1, 1));
            }
            //r.line(glm::vec3(-5, 0, 0), glm::vec3(5, 0, 0), glm::vec4(1, 0, 0, 1));
            r.render(cam);

            //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            //SDL_RenderClear(renderer);

            //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            //// debug flow field
            //for (int x = 0; x < flowField.width(); ++x) {
            //    for (int y = 0; y < flowField.height(); ++y) {
            //        vec2 v = flowField.get(x, y);
            //        render_direction(renderer, flowField.cell_center(x, y), v, flowField.cell_size());
            //    }
            //}

            //// debug path
            //SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            //for (int i = 0; i < agentPath.path_points().size(); ++i) {
            //    vec2 pt1 = agentPath.path_points()[i];
            //    vec2 pt2 = agentPath.path_points()[(i + 1) % agentPath.path_points().size()];
            //    int x1, y1, x2, y2;
            //    world_to_screen(pt1.x, pt1.y, x1, y1);
            //    world_to_screen(pt2.x, pt2.y, x2, y2);
            //    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            //    render_circle(renderer, x1, y1, 8);
            //}

            ////{
            ////    vec2 dir;
            ////    vec2 nearest = agentPath.nearest(agents[0].future, dir);
            ////    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
            ////    int sx, sy;
            ////    world_to_screen(nearest.x, nearest.y, sx, sy);
            ////    render_circle(renderer, sx, sy, 8);
            ////}

            //for (const auto& agent : agents) {
            //    SDL_Rect rect = {
            //        (int)((agent.position.x - 0.5f) * 32 + 640), (int)((agent.position.y - 0.5f) * 32 + 360),
            //        32, 32
            //    };

            //    int px, py, fx, fy, tx, ty;
            //    world_to_screen(agent.position.x, agent.position.y, px, py);
            //    world_to_screen(agent.future.x, agent.future.y, fx, fy);
            //    world_to_screen(agent.target.x, agent.target.y, tx, ty);

            //    //SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            //    //SDL_RenderDrawLine(renderer, px, py, fx, fy);
            //    //SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            //    //SDL_RenderDrawLine(renderer, fx, fy, tx, ty);
            //    //render_circle(renderer, fx, fy, (int)agent.projectionRadius * 32);

            //    SDL_RenderCopyEx(renderer, dudeTexture, nullptr, &rect, agent.rotation, nullptr, SDL_FLIP_NONE);
            //}

            //// render FPS
            //{
            //    char buffer[256] = { 0 };
            //    snprintf(buffer, 16, "FPS: %d", fps);
            //    SDL_Color white = { 255, 255, 255, 255 };
            //    SDL_Surface* fpsTextSurface = TTF_RenderText_Solid(debugFont, buffer, white);
            //    SDL_Texture* fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsTextSurface);

            //    SDL_Rect destRect = { 0, 0, 100, 20 };
            //    SDL_RenderCopy(renderer, fpsTexture, nullptr, &destRect);

            //    SDL_DestroyTexture(fpsTexture);
            //    SDL_FreeSurface(fpsTextSurface);
            //}

            //SDL_RenderPresent(renderer);
        }
    }

    //SDL_DestroyRenderer(renderer);
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

    int x0, y0, x1, y1;
    world_to_screen(p0.x, p0.y, x0, y0);
    world_to_screen(p1.x, p1.y, x1, y1);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(renderer, x0, y0, x1, y1);
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderDrawPoint(renderer, x1, y1);
}

void world_to_screen(f32 wx, f32 wy, int& sx, int& sy) {
    sx = (int)(wx * 32) + 640;
    sy = (int)(wy * 32) + 360;
}

void screen_to_world(int sx, int sy, f32& wx, f32& wy) {
    wx = (sx - 640) / 32.f;
    wy = (sy - 360) / 32.f;
}