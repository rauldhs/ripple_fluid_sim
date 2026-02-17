#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "particle_renderer.h"

int main() {
    auto renderer = ParticleRenderer();
    std::vector<Particle> particles = {Particle{{0, 0, 0}}};

    renderer.update_particles(particles);

    while (!renderer.should_close()) {
        renderer.draw();
    }
}
