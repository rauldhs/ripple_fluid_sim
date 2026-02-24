#include "app/app.hpp"

#include <chrono>

#include "engine/core/window.hpp"
#include "engine/rendering/particle_renderer.hpp"

App::App(const AppSpecification& app_spec)
    : app_spec(std::move(app_spec)), window(app_spec.height, app_spec.width, app_spec.name), ui(window.get_handle()) {
    simulation.regenerate_particles(particles, particle_renderer.radius * 8.0f);

    window.add_resize_listener([this](int w, int h) { camera.set_aspect_ratio(w, h); });
    window.add_resize_listener([this](int, int) { particle_renderer.update_proj_matrix(camera.render_data); });
    window.add_cursor_pos_listener([this](double x, double y) { input_manager.update_mouse_pos(x, y); });
    window.add_key_action_listener([this](int key, int action) { input_manager.update_key(key, action); });

    ui.add_reset_button_listener(
        [this](void) -> void { simulation.regenerate_particles(particles, particle_renderer.radius * 8.0f); });

    camera.set_aspect_ratio(app_spec.width, app_spec.height);
    particle_renderer.update_proj_matrix(camera.render_data);
}

App::App() : App(AppSpecification()) {}

void App::run() {
    while (!window.should_close()) {
        auto start = std::chrono::high_resolution_clock::now();

        simulation.update_particles(particles, particle_renderer.radius);

        particle_renderer.update_particles(particles);
        particle_renderer.draw(camera.render_data);

        ui.draw(simulation.simulation_data,
                std::format("FPS {} ({}) , total particles: {}", 1 / delta_time, delta_time * 1000, particles.size()));
        window.update(input_manager.input_state);

        camera.update(input_manager.input_state, delta_time);

        auto end = std::chrono::high_resolution_clock::now();
        delta_time = std::chrono::duration<float>(end - start).count();
    }
}
