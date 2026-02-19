#include <glm/glm.hpp>
#include <random>

#include "kernels.h"
#include "particle_renderer.h"

class SphSimulation {
   private:
    std::default_random_engine generator;
    void apply_bounding_box_physics(std::vector<Particle> &particles, float radius, float energy_loss_factor,
                                    float ground_friction);
    glm::vec3 get_pressure_force(std::vector<Particle> &particles, Particle &p_i);
    glm::vec3 get_viscosity_force(std::vector<Particle> &particles, Particle &p_i);

   public:
    // TODO: temp public parameters
    float GAS_CONSTANT = 200000.0f;
    float REST_DENSITY = 0.000001f;
    float VISCOSITY = 0.5f;
    float GRAVITY = 1000.0f;
    float H_SMOOTHING = 16.0f;
    float DT = 0.007f;

    float ENERGY_LOSS = 0.8f;
    float FRICTION = 0.95f;
    glm::vec3 BOX_START = {0.0f, 0.0f, 0.0f};
    glm::vec3 BOX_END = {200.0f, 100.0f, 100.0f};

    SphSimulation();
    void update_particles(std::vector<Particle> &particles, float radius);
    void regenerate_particles(std::vector<Particle> &particles, float particle_spacing);
};
