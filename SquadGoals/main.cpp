#include <vector>
#include <random>
#include <iostream>
#include <cstdlib>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <Box2D/Box2D.h>

#include <effolkronium/random.hpp>

#include "algebra.h"
#include "perlin.h"
#include "flowfield.h"
#include "box2dSdlDebugDraw.h"
#include "path.h"
#include "input_state.h"
#include "renderer.h"
#include "flatdraw.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

enum class agent_seek_mode {
    kWander,
    kFollowPath,
    kCount,
};

const char* seek_mode_strs[(int)agent_seek_mode::kCount] {
    "wander",
    "follow path",
};

struct debug_config {
    bool showWanderProjection = false;
    bool showTarget = false;
    bool showSeparationRadius = false;
    bool showPath = false;
};

struct agent_config {
    agent_seek_mode seekMode = agent_seek_mode::kFollowPath;
    
    f32 maxSpeed = 10.f;
    f32 maxAccel = 5.f;
    
    f32 wanderProjectionDist = 4.f;
    f32 wanderProjectionRadius = 1.f;
    
    f32 wanderAngleRange = 10.f;
    f32 wanderInterval = 1 / 60.f;
    
    f32 separationDist = 2.f;

    f32 movementScalar = 1.f;
    f32 flowScalar = 0.f;
    f32 separationScalar = 0.1f;

    f32 pathFollowDist = 0.5f;
};

struct steer_agent {
    vec2 position = vec2::ZERO;
    vec2 velocity = vec2::ZERO;
    vec2 acceleration = vec2::ZERO;
    f32 rotation = 0;

    vec2 target = vec2::ZERO;

    vec2 future;
    f32 wanderAngle;
    f32 wanderTimer = 0.f;

    b2Body* body;
};

static void init_agent(steer_agent& ag, b2World& world) {
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

int main(int argc, char* argv[]) {
    Random::seed(1);
    input_state input;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    TTF_Font* debugFont = TTF_OpenFont("assets/prstartk.ttf", 16);

    SDL_Window* window = SDL_CreateWindow("Squad Goals", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_OPENGL);
    
    renderer r;
    r.init(window);

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplSDL2_InitForOpenGL(window, r.glContext());
    ImGui_ImplOpenGL3_Init(r.glslVersion());

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    flat_draw_context draw(r);

    camera cam;
    //cam.position = glm::vec3(0.f, 20.f, 0.f);

    std::vector<vec2> pathPts;
    const int ptCount = 30;
    const f32 delta = 360.f / ptCount;
    const f32 radX = 20, radY = 10;
    for (int i = 0; i < ptCount; ++i) {
        f32 a = delta * i;
        f32 b = delta * (i + 1);
        pathPts.push_back(vec2(math::cos(a) * radX, math::sin(a) * radY));
        pathPts.push_back(vec2(math::cos(b) * radX, math::sin(b) * radY));
    }

    path agentPath(path_dir::kCW, pathPts.data(), pathPts.size());

    b2World world(vec2::ZERO);

    //box2dSdlDebugDraw sdlDebugDraw(renderer);
    //world.SetDebugDraw(&sdlDebugDraw);

    const i32 velocityIterations = 8;
    const i32 positionIterations = 8;

    /*SDL_Surface* dudeSurface = IMG_Load("assets/dude.png");
    SDL_Texture* dudeTexture = SDL_CreateTextureFromSurface(renderer, dudeSurface);*/

    debug_config debugConfig;
    agent_config agentConfig;

    std::vector<steer_agent> agents;
    flow_field flowField(40.f, 36.f, 1.f, -20.f, -18.f);

    perlin_gen perlin(10000);
    flowField.perlin_angles(perlin, 1.f);

    const int AGENT_COUNT = 40;

    for (int i = 0; i < AGENT_COUNT; ++i) {
        steer_agent ag;
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
            ImGui_ImplSDL2_ProcessEvent(&event);
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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        input.update();

        int mx, my;
        u32 mb = SDL_GetRelativeMouseState(&mx, &my);

        

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

        // UI
        {
            ImGui::Begin("Camera");

            ImGui::InputFloat("theta", &cam.theta);
            ImGui::InputFloat("phi", &cam.phi);
            ImGui::InputFloat("rho", &cam.rho);

            ImGui::InputFloat3("position", &cam.position[0], 2);

            ImGui::End();
        }
        {
            ImGui::Begin("Agents");

            ImGui::Text("Seek Mode: ");
            ImGui::SameLine();

            int seekModeIndex = (int)agentConfig.seekMode;
            if (ImGui::Button(seek_mode_strs[seekModeIndex])) {
                seekModeIndex++;
                if (seekModeIndex >= (int)agent_seek_mode::kCount) {
                    seekModeIndex = 0;
                }
                agentConfig.seekMode = (agent_seek_mode)seekModeIndex;
            }

            /*f32 maxSpeed = 10.f;
            f32 maxAccel = 5.f;
            f32 projectionDist = 4.f;
            f32 projectionRadius = 1.f;
            f32 wanderMult = 10.f;
            f32 wanderInterval = 1 / 60.f;*/

            ImGui::InputFloat("Max Speed", &agentConfig.maxSpeed, 0.1f, 1.f, 2);
            ImGui::InputFloat("Max Acceleration", &agentConfig.maxAccel, 0.1f, 1.f, 2);

            ImGui::InputFloat("Separation Dist", &agentConfig.separationDist, 0.1f, 1.f, 2);

            ImGui::InputFloat("Movement Scalar", &agentConfig.movementScalar, 0.1f, 1.f, 2);
            ImGui::InputFloat("Flow Scalar", &agentConfig.flowScalar, 0.1f, 1.f, 2);
            ImGui::InputFloat("Separation Scalar", &agentConfig.separationScalar, 0.1f, 1.f, 2);

            ImGui::InputFloat("Path Distance", &agentConfig.pathFollowDist, 0.1f, 1.f, 2);


            /*f32 wanderAngleRange = 10.f;
            f32 wanderInterval = 1 / 60.f;*/

            ImGui::Separator();

            ImGui::InputFloat("Wander Proj Dist", &agentConfig.wanderProjectionDist, 0.1f, 1.f, 2);
            ImGui::InputFloat("Wander Proj Radius", &agentConfig.wanderProjectionRadius, 0.1f, 1.f, 2);
            ImGui::InputFloat("Wonder Angle Range", &agentConfig.wanderAngleRange, 0.1f, 1.f, 2);

            int ms = (int)(agentConfig.wanderInterval * 1000.f);
            ImGui::InputInt("Wonder Drift Interval (ms)", &ms);
            agentConfig.wanderInterval = ms / 1000.f;

            ImGui::End();
        }
        {
            ImGui::Begin("Debug");

            ImGui::Checkbox("Wander Projection", &debugConfig.showWanderProjection);
            ImGui::Checkbox("Target", &debugConfig.showTarget);
            ImGui::Checkbox("Separation Radius", &debugConfig.showSeparationRadius);
            ImGui::Checkbox("Path", &debugConfig.showPath);

            ImGui::End();
        }
        {
            ImGui::Begin("Stats");
            
            ImGui::Text("FPS: %d", fps);

            ImGui::End();
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

                if (input.get_key(SDL_SCANCODE_Q)) {
                    cam.rho += dt;
                }

                if (input.get_key(SDL_SCANCODE_E)) {
                    cam.rho -= dt;
                }

                if ((mb & 4) != 0) {
                    f32 pitch = -my / 8.f;
                    f32 yaw = mx / 8.f;
                    cam.orbit(pitch, yaw);
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
                {
                    auto wander = [&agentConfig, dt](steer_agent& agent) -> void {
                        if (agent.position.len2() > 10000) {
                            agent.target = vec2::ZERO;
                            return;
                        }

                        vec2 velocity = agent.velocity;
                        if (agent.velocity == vec2::ZERO) {
                            velocity = math::vec2_from_angle(Random::get(0.f, 360.f));
                        }

                        agent.future = agent.position + vec2::normalize(velocity) * agentConfig.wanderProjectionDist;

                        agent.wanderTimer -= dt;
                        if (agent.wanderTimer <= 0) {
                            agent.wanderAngle += Random::get<f32>(-agentConfig.wanderAngleRange, agentConfig.wanderAngleRange);
                            agent.wanderTimer += agentConfig.wanderInterval;
                        }

                        agent.target.x = agent.future.x + math::cos(agent.wanderAngle) * agentConfig.wanderProjectionRadius;
                        agent.target.y = agent.future.y + math::sin(agent.wanderAngle) * agentConfig.wanderProjectionRadius;
                    };

                    vec2 velocity = agent.velocity;

                    vec2 predicted = agent.position + vec2::normalize(agent.velocity) * 2.f;

                    vec2 pathDir;
                    vec2 nearest = agentPath.nearest(predicted, pathDir);
                    f32 pathDist = vec2::dist(nearest, agent.position);

                    switch (agentConfig.seekMode) {
                    case agent_seek_mode::kWander:
                        wander(agent);
                        break;
                    case agent_seek_mode::kFollowPath:
                        if (pathDist > agentConfig.pathFollowDist) {
                            agent.target = nearest + pathDir * 1.f;
                            agent.future = predicted;
                        }
                        else {
                            wander(agent);
                        }
                        break;
                    }
                }

                // SEPARATE
                vec2 separation = [&agent, &agents, &agentConfig]() -> vec2 {
                    vec2 sum = vec2::ZERO;
                    int count = 0;
                    for (auto& other : agents) {
                        if (&other == &agent) {
                            continue;
                        }

                        vec2 delta = agent.position - other.position;
                        f32 d = delta.len();
                        if (d > 0 && d < agentConfig.separationDist) {
                            sum += vec2::normalize(delta) / d;
                            ++count;
                        }
                    }

                    if (count > 0) {
                        sum /= count;
                        sum.normalize();
                        sum *= agentConfig.maxSpeed;
                        return sum;
                    }
                    else {
                        return vec2::ZERO;
                    }
                }();

                // SEEK
                [&agent, &flowField, &agentPath, &separation, &agentConfig, dt] {
                    vec2 targetDir;
                    f32 targetDist;

                    vec2 targetDelta = agent.target - agent.position;

                    targetDelta.decompose(targetDir, targetDist);

                    vec2 desired;
                    const f32 BORDER_SIZE = 128.f;

                    vec2 flow = flowField.get(agent.position);
                    vec2 movement = targetDir * math::clamp01(targetDist / 40);

                    desired = flow + movement;

                    desired = agentConfig.movementScalar * movement + agentConfig.flowScalar * flow + agentConfig.separationScalar * separation;
                    desired.normalize();
                    desired *= agentConfig.maxSpeed;

                    vec2 steer = desired - agent.velocity;
                    steer.limit(agentConfig.maxAccel);

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
            // origin handle
            r.line(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec4(1, 0, 0, 1));
            r.line(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec4(0, 1, 0, 1));
            r.line(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec4(0, 0, 1, 1));

            //// debug path
            if (debugConfig.showPath) {
                draw.set_color(1, 1, 0);
                for (int i = 0; i < agentPath.path_points().size(); ++i) {
                    vec2 pt1 = agentPath.path_points()[i];
                    vec2 pt2 = agentPath.path_points()[(i + 1) % agentPath.path_points().size()];
                    draw.line(pt1, pt2);
                }
            }

            for (const auto& agent : agents) {
                vec2 points[3] = {
                    .5f * math::vec2_from_angle(agent.rotation) + agent.position,
                    .5f * math::vec2_from_angle(agent.rotation - 135) + agent.position,
                    .5f * math::vec2_from_angle(agent.rotation + 135) + agent.position
                };

                draw.set_color(0, 1, 0);
                draw.lines(points, 3);

                if (debugConfig.showWanderProjection) {
                    draw.set_color_bytes(179, 120, 210);
                    draw.line(agent.position, agent.future);
                    draw.line(agent.future, agent.future + math::vec2_from_angle(agent.wanderAngle) * agentConfig.wanderProjectionRadius);
                    draw.circle(agent.future, agentConfig.wanderProjectionRadius);
                }

                if (debugConfig.showTarget) {
                    draw.set_color_bytes(31, 255, 31);
                    draw.line(agent.position, agent.target);
                }

                if (debugConfig.showSeparationRadius) {
                    draw.set_color_bytes(100, 149, 247);
                    draw.circle(agent.position, agentConfig.separationDist);
                }
            }

            ImGui::Render();
            SDL_GL_MakeCurrent(window, r.glContext());

            r.render(cam);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            SDL_GL_SwapWindow(window);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    r.shutdown();

    //SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
