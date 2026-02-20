#include "app.hpp"

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include "engine/core/window.hpp"
#include "engine/rendering/particle_renderer.hpp"

App::App(const AppSpecification& app_spec)
    : app_spec(std::move(app_spec)),
      input_manager(std::make_shared<InputManager>()),
      window(app_spec.height, app_spec.width, app_spec.name, input_manager) {}

App::App() : App(AppSpecification()) {}

void App::run() {
    simulation.regenerate_particles(particles, particle_spacing);

    while (!window.should_close()) {
        simulation.update_particles(particles, particle_renderer.particle_radius);
        particle_renderer.update_particles(particles);
        particle_renderer.draw(camera.render_data);

        window.update(input_manager->input_state);

        camera.update(input_manager->input_state);
    }
}
