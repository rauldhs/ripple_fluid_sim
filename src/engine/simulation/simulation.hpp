#pragma once

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
    float VISCOSITY = 0.5f;
    float GRAVITY = 1000.0f;
    float H_SMOOTHING = 16.0f;
    float DT = 0.007f;

    float ENERGY_LOSS = 0.8f;
    float FRICTION = 0.95f;

    glm::vec3 BOX_START = {-300.0f, -100.0f, -100.0f};
    glm::vec3 BOX_END = {300.0f, 100.0f, 100.0f};
};

class SphSimulation {
   private:
    std::default_random_engine generator;
    void apply_bounding_box_physics(std::vector<Particle> &particles, float radius, float energy_loss_factor,
                                    float ground_friction);
    glm::vec3 get_pressure_force(std::vector<Particle> &particles, Particle &p_i);
    glm::vec3 get_viscosity_force(std::vector<Particle> &particles, Particle &p_i);
    std::vector<size_t> &get_neighbors(const Particle &p);
    std::unordered_map<std::tuple<int, int, int>, std::vector<size_t>, TupleHash> particle_grid;

   public:
    SphSimulationData simulation_data;

    void update_particles(std::vector<Particle> &particles, float radius);
    void regenerate_particles(std::vector<Particle> &particles, float particle_spacing);
};
