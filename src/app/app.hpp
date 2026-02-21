#pragma once

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>
// clang-format on

#include "engine/core/input_manager.hpp"
#include "engine/core/window.hpp"
#include "engine/rendering/camera.hpp"
#include "engine/rendering/particle_renderer.hpp"
#include "engine/rendering/ui.hpp"
#include "engine/simulation/simulation.hpp"

struct GlfwContext {
    GlfwContext() { glfwInit(); }
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
    float delta_time = 0.016;

    void prepare_imgui();

   public:
    App();
    App(const AppSpecification& app_spec);

    void run();
};
