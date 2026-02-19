#include <fenv.h>
#include <unistd.h>

#include <glm/glm.hpp>
#include <vector>

#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "particle_renderer.h"
#include "simulation.h"

int main() {
    ParticleRenderer renderer = ParticleRenderer();
    SphSimulation simulation = SphSimulation();
    std::vector<Particle> particles;

    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
    float particle_spacing = renderer.particle_radius * 2.0f;

    simulation.regenerate_particles(particles, particle_spacing);

    while (!renderer.should_close()) {
        simulation.update_particles(particles, renderer.particle_radius);
        renderer.update_particles(particles);
        renderer.draw();

        glfwSwapBuffers(renderer.get_window());
        glfwPollEvents();
    }
}
