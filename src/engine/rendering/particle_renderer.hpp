#pragma once

#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>
#include <vector>

#include "engine/rendering/camera.hpp"
#include "engine/rendering/particle.hpp"

class ParticleRenderer {
   private:
    unsigned int VAO, VBO, instanceVBO, instanceVelocityVBO, EBO;

    int proj_uniform_location, view_uniform_location, model_uniform_location;

    unsigned int SHADER_PROGRAM;

    glm::mat4 model = glm::mat4(1);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    unsigned int current_total_particles = 0, particle_buffer_capacity = 0;

    void generate_mesh(float radius, unsigned int sectorCount, unsigned int stackCount);

    void initialize_buffers();
    void initialize_shader_program();

   public:
    float radius = 4.0f;

    ParticleRenderer();
    ~ParticleRenderer();

    void update_particles(std::vector<Particle>& particles);
    void update_proj_matrix(CameraRenderData camera_render_data);

    void draw(const CameraRenderData& camera_render_data);
};
