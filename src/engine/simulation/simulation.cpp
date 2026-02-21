#include "engine/simulation/simulation.hpp"

#include <cmath>
#include <cstddef>
#include <unordered_map>
#include <vector>

#include "engine/rendering/particle.hpp"
#include "engine/simulation/kernels.hpp"
#include "glm/fwd.hpp"

void SphSimulation::apply_bounding_box_physics(std::vector<Particle> &particles, float radius, float energy_loss_factor,
                                               float ground_friction) {
    for (auto &p : particles) {
        if (p.pos.y <= simulation_data.BOX_START.y + radius) {
            p.pos.y = simulation_data.BOX_START.y + radius;
            p.velocity.y *= -energy_loss_factor;
            p.velocity.x *= ground_friction;
        }
        if (p.pos.x <= simulation_data.BOX_START.x + radius) {
            p.pos.x = simulation_data.BOX_START.x + radius;
            p.velocity.x *= -energy_loss_factor;
        }
        if (p.pos.x >= simulation_data.BOX_END.x - radius) {
            p.pos.x = simulation_data.BOX_END.x - radius;
            p.velocity.x *= -energy_loss_factor;
        }
        if (p.pos.y >= simulation_data.BOX_END.y - radius) {
            p.pos.y = simulation_data.BOX_END.y - radius;
            p.velocity.y *= -energy_loss_factor;
        }
        if (p.pos.z <= simulation_data.BOX_START.z + radius) {
            p.pos.z = simulation_data.BOX_START.z + radius;
            p.velocity.z *= -energy_loss_factor;
        }
        if (p.pos.z >= simulation_data.BOX_END.z - radius) {
            p.pos.z = simulation_data.BOX_END.z - radius;
            p.velocity.z *= -energy_loss_factor;
        }
    }
}

glm::vec3 SphSimulation::get_pressure_force(std::vector<Particle> &particles, Particle &p_i) {
    glm::vec3 sum(0);
    for (size_t i : get_neighbors(p_i)) {
        if (&p_i != &particles[i]) {
            float dj = std::max(particles[i].density, 1e-6f);
            sum += particles[i].mass * ((p_i.pressure + particles[i].pressure) / (2 * dj)) *
                   grad_spiky(p_i.pos - particles[i].pos, simulation_data.H_SMOOTHING);
        }
    }
    return -sum;
}

glm::vec3 SphSimulation::get_viscosity_force(std::vector<Particle> &particles, Particle &p_i) {
    glm::vec3 sum(0);
    for (size_t i : get_neighbors(p_i)) {
        if (&p_i != &particles[i]) {
            float dj = std::max(particles[i].density, 1e-6f);
            sum += particles[i].mass * ((particles[i].velocity - p_i.velocity) / dj) *
                   laplacian_viscosity(p_i.pos - particles[i].pos, simulation_data.H_SMOOTHING);
        }
    }
    return simulation_data.VISCOSITY * sum;
}

std::vector<size_t> &SphSimulation::get_neighbors(const Particle &p) {
    auto grid_pos = std::make_tuple(std::floor(p.pos.x / simulation_data.H_SMOOTHING),
                                    std::floor(p.pos.y / simulation_data.H_SMOOTHING),
                                    std::floor(p.pos.z / simulation_data.H_SMOOTHING));
    return particle_grid[grid_pos];
}

void SphSimulation::update_particles(std::vector<Particle> &particles, float radius) {
    particle_grid.clear();

    for (size_t i = 0; i < particles.size(); i++) {
        auto grid_pos = std::make_tuple(std::floor(particles[i].pos.x / simulation_data.H_SMOOTHING),
                                        std::floor(particles[i].pos.y / simulation_data.H_SMOOTHING),
                                        std::floor(particles[i].pos.z / simulation_data.H_SMOOTHING));
        particle_grid[grid_pos].push_back(i);
    }

    for (auto &p : particles) {
        p.density = 0;
        for (size_t i : get_neighbors(p)) {
            p.density += particles[i].mass * poly6_kernel(p.pos - particles[i].pos, simulation_data.H_SMOOTHING);
        }
        p.pressure = simulation_data.GAS_CONSTANT * (p.density - simulation_data.REST_DENSITY);
    }

    for (auto &p : particles) {
        glm::vec3 total_forces = get_pressure_force(particles, p) + get_viscosity_force(particles, p) +
                                 p.density * glm::vec3(0.0f, -simulation_data.GRAVITY, 0.0f);

        float di = std::max(p.density, 1e-6f);
        p.accerelation = total_forces / di;
    }

    for (auto &p : particles) {
        p.velocity += p.accerelation * simulation_data.DT;
        p.pos += p.velocity * simulation_data.DT;
    }

    SphSimulation::apply_bounding_box_physics(particles, radius, simulation_data.ENERGY_LOSS, simulation_data.FRICTION);
}

void SphSimulation::regenerate_particles(std::vector<Particle> &particles, float particle_spacing) {
    glm::vec3 current_center = {0, 0, 0};

    float spawn_w = (simulation_data.BOX_END.x - simulation_data.BOX_START.x) * 0.4f;
    float spawn_h = (simulation_data.BOX_END.y - simulation_data.BOX_START.y) * 0.6f;

    float spawn_left = current_center.x - (spawn_w / 2.0f);
    float spawn_bottom = current_center.y - (spawn_h / 2.0f);

    float spawn_depth = (simulation_data.BOX_END.z - simulation_data.BOX_START.z) * 0.4f;
    float spawn_back = current_center.z - (spawn_depth / 2.0f);

    std::uniform_real_distribution<float> distribution(-particle_spacing * 0.15f, particle_spacing * 0.15f);

    particles.clear();
    for (float z = spawn_back; z < spawn_back + spawn_depth; z += particle_spacing) {
        for (float y = spawn_bottom; y < spawn_bottom + spawn_h; y += particle_spacing) {
            for (float x = spawn_left; x < spawn_left + spawn_w; x += particle_spacing) {
                float jx = x + distribution(generator);
                float jy = y + distribution(generator);
                float jz = z + distribution(generator);

                particles.push_back(Particle{{jx, jy, jz}});
            }
        }
    }
}
