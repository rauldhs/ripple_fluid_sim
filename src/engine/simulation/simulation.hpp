#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <random>
#include <vector>

#include "engine/rendering/particle.hpp"

struct TupleHash {
    std::size_t operator()(const std::tuple<int, int, int> &pos) const {
        int p1 = 73856093, p2 = 19349663, p3 = 83492791;
        int hash = (p1 * std::get<0>(pos)) ^ (p2 * std::get<1>(pos)) ^ (p3 * std::get<2>(pos));
        return static_cast<std::size_t>(hash);
    }
};

struct SphSimulationData {
    float GAS_CONSTANT = 200000.0f;
    float REST_DENSITY = 0.000001f;
    float VISCOSITY = 5.0f;
    float GRAVITY = 1000.0f;
    float H_SMOOTHING = 32.0f;
    float DT = 0.007f;

    float ENERGY_LOSS = 0.8f;
    float FRICTION = 0.95f;

    float TENSION_COEFICIENT = 0.001f;

    glm::vec3 BOX_START = {-500.0f, -100.0f, -300.0f};
    glm::vec3 BOX_END = {500.0f, 1000.0f, 500.0f};
};

class SphSimulation {
   private:
    std::default_random_engine generator;
    unsigned int step_count = 0;
    std::vector<std::vector<size_t>> neighbors_cache;
    std::unordered_map<std::tuple<int, int, int>, std::vector<size_t>, TupleHash> particle_grid;

    void apply_bounding_box_physics(std::vector<Particle> &particles, float radius, float energy_loss_factor,
                                    float ground_friction);
    glm::vec3 get_pressure_force(std::vector<Particle> &particles, size_t index);
    glm::vec3 get_viscosity_force(std::vector<Particle> &particles, size_t index);
    glm::vec3 get_surface_tension_force(std::vector<Particle> &particles, size_t i);
    void get_neighbors(const Particle &p, std::vector<size_t> &result);
    void z_sort_particles(std::vector<Particle> &particles);

    uint32_t expand_bits(uint32_t x);

    // NOTE: the ints must be at most 10 bits long,the rest is used for interleaving
    // this is some black magic shit, basically makes the bits be like x1y1z1 x2y2z2....
    uint32_t morton_code(uint32_t x, uint32_t y, uint32_t z);
    void z_insertion_sort(std::vector<std::pair<uint32_t, size_t>> &vec);

   public:
    SphSimulationData simulation_data;

    void update_particles(std::vector<Particle> &particles, float radius);
    void regenerate_particles(std::vector<Particle> &particles, float particle_spacing);
};
