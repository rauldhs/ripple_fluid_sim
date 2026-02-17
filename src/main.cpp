#include <glm/glm.hpp>
#include <iostream>
#include <random>
#include <vector>

#include "particle_renderer.h"

int main() {
    auto renderer = ParticleRenderer();
    std::vector<Particle> particles;
    particles.reserve(10000);
    std::mt19937 rng(42);
    std::normal_distribution<float> thickness(0, 0.05f);
    std::normal_distribution<float> spread(0, 0.15f);
    for (int i = 0; i < 10000; i++) {
        int arm = i % 3;
        float t = std::uniform_real_distribution<float>(0, 1)(rng);
        float angle = t * 4 * glm::pi<float>() + arm * (2 * glm::pi<float>() / 3);
        float r = t * 0.9f + spread(rng);
        particles.push_back(Particle{{r * cos(angle), thickness(rng), r * sin(angle)}});
    }

    renderer.update_particles(particles);

    while (!renderer.should_close()) {
        renderer.draw();
    }
}
