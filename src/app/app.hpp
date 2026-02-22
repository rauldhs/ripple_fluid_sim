#pragma once

#include <memory>

#include "engine/core/input_manager.hpp"
#include "engine/core/window.hpp"
#include "engine/rendering/camera.hpp"
#include "engine/rendering/particle_renderer.hpp"
#include "engine/rendering/ui.hpp"
#include "engine/simulation/simulation.hpp"

struct GlfwContext {
    GlfwContext() {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
        glfwInit();
    }
    ~GlfwContext() { glfwTerminate(); }
};

struct AppSpecification {
    int width = 800, height = 600;
    std::string name = "App";
};

class App {
   private:
    AppSpecification app_spec;
    GlfwContext glfw_context;

    std::shared_ptr<InputManager> input_manager;
    Window window;

    Ui ui;

    ParticleRenderer particle_renderer;

    SphSimulation simulation;
    Camera camera;

    std::vector<Particle> particles;
    float delta_time = 0.016f;

    void prepare_imgui();

   public:
    App();
    explicit App(const AppSpecification& app_spec);

    void run();
};
