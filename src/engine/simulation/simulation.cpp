#include "engine/simulation/simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
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

glm::vec3 SphSimulation::get_pressure_force(std::vector<Particle> &particles, size_t i) {
    glm::vec3 sum(0);
    auto &p_i = particles[i];

    for (size_t j : neighbors_cache[i]) {
        if (&p_i != &particles[j]) {
            float dj = std::max(particles[j].density, 1e-6f);
            sum += particles[j].mass * ((p_i.pressure + particles[j].pressure) / (2 * dj)) *
                   grad_spiky(p_i.pos - particles[j].pos, simulation_data.H_SMOOTHING);
        }
    }
    return -sum;
}

glm::vec3 SphSimulation::get_viscosity_force(std::vector<Particle> &particles, size_t i) {
    auto &p_i = particles[i];
    glm::vec3 sum(0);
    for (size_t j : neighbors_cache[i]) {
        if (&p_i != &particles[j]) {
            float dj = std::max(particles[j].density, 1e-6f);
            sum += particles[j].mass * ((particles[j].velocity - p_i.velocity) / dj) *
                   laplacian_viscosity(p_i.pos - particles[j].pos, simulation_data.H_SMOOTHING);
        }
    }
    return simulation_data.VISCOSITY * sum;
}

void SphSimulation::get_neighbors(const Particle &p, std::vector<size_t> &result) {
    result.clear();
    for (int i : {-1, 0, 1}) {
        for (int j : {-1, 0, 1}) {
            for (int k : {-1, 0, 1}) {
                auto grid_pos = std::make_tuple(std::floor(p.pos.x / simulation_data.H_SMOOTHING) + i,
                                                std::floor(p.pos.y / simulation_data.H_SMOOTHING) + j,
                                                std::floor(p.pos.z / simulation_data.H_SMOOTHING) + k);
                auto &neighbors = particle_grid[grid_pos];
                result.insert(result.end(), neighbors.begin(), neighbors.end());
            }
        }
    }
}

uint32_t SphSimulation::expand_bits(uint32_t x) {
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;
    return x;
}

uint32_t SphSimulation::morton_code(uint32_t x, uint32_t y, uint32_t z) {
    // https://stackoverflow.com/questions/1024754/how-to-compute-a-3d-morton-number-interleave-the-bits-of-3-ints
    // http://www.graphics.stanford.edu/~seander/bithacks.html#InterleaveBMN
    return expand_bits(x) | expand_bits(y) << 1 | expand_bits(z) << 2;
}

void SphSimulation::z_insertion_sort(std::vector<std::pair<uint32_t, size_t>> &vec) {
    for (auto it = vec.begin(); it != vec.end(); it++) {
        auto const insertion_point =
            std::upper_bound(vec.begin(), it, *it, [](auto &a, auto &b) { return a.first < b.first; });

        std::rotate(insertion_point, it, it + 1);
    }
}

void SphSimulation::z_sort_particles(std::vector<Particle> &particles) {
    std::vector<std::pair<uint32_t, size_t>> handles(particles.size());
    for (size_t i = 0; i < particles.size(); i++) {
        uint32_t x = static_cast<uint32_t>(
            (particles[i].pos.x - std::min(simulation_data.BOX_START.x, simulation_data.BOX_END.x)) /
            simulation_data.H_SMOOTHING);
        uint32_t y = static_cast<uint32_t>(
            (particles[i].pos.y - std::min(simulation_data.BOX_START.y, simulation_data.BOX_END.y)) /
            simulation_data.H_SMOOTHING);
        uint32_t z = static_cast<uint32_t>(
            (particles[i].pos.z - std::min(simulation_data.BOX_START.z, simulation_data.BOX_END.z)) /
            simulation_data.H_SMOOTHING);
        handles[i] = {morton_code(x, y, z), i};
    }
    z_insertion_sort(handles);

    std::vector<Particle> sorted_particles(particles.size());
    for (size_t i = 0; i < particles.size(); i++) {
        sorted_particles[i] = particles[handles[i].second];
    }
    particles = std::move(sorted_particles);
}

void SphSimulation::update_particles(std::vector<Particle> &particles, float radius) {
    particle_grid.clear();
    particle_grid.reserve(particles.size());

    if (step_count % 100 == 0) {
        // this problably has very small performance gains since its using a hashmap instead of just a giant uniform
        // grid
        z_sort_particles(particles);
    }
    step_count++;

    for (size_t i = 0; i < particles.size(); i++) {
        auto grid_pos = std::make_tuple(std::floor(particles[i].pos.x / simulation_data.H_SMOOTHING),
                                        std::floor(particles[i].pos.y / simulation_data.H_SMOOTHING),
                                        std::floor(particles[i].pos.z / simulation_data.H_SMOOTHING));
        particle_grid[grid_pos].push_back(i);
    }

    for (size_t i = 0; i < particles.size(); i++) {
        get_neighbors(particles[i], neighbors_cache[i]);
    }

    for (size_t i = 0; i < particles.size(); i++) {
        auto &p = particles[i];
        p.density = 0;
        for (size_t j : neighbors_cache[i]) {
            p.density += particles[j].mass * poly6_kernel(p.pos - particles[j].pos, simulation_data.H_SMOOTHING);
        }
        p.pressure = simulation_data.GAS_CONSTANT * (p.density - simulation_data.REST_DENSITY);
    }

    for (size_t i = 0; i < particles.size(); i++) {
        auto &p = particles[i];
        glm::vec3 total_forces = get_pressure_force(particles, i) + get_viscosity_force(particles, i) +
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

    neighbors_cache.resize(particles.size() + 1);
}
