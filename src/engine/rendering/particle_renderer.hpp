#pragma once

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/vec3.hpp>
#include <vector>

#include "engine/rendering/camera.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

struct Particle {
    glm::vec3 pos;
    glm::vec3 velocity = {0, 0, 0};
    glm::vec3 accerelation = {0, 0, 0};
    float mass = 10.0f;
    float pressure = 0.0f;
    float density = 0.0f;
};

class ParticleRenderer {
   private:
    unsigned int VAO, VBO, instanceVBO, instanceVelocityVBO, EBO;

    int proj_uniform_location, view_uniform_location, model_uniform_location;

    unsigned int SHADER_PROGRAM;

    glm::mat4 model = glm::mat4(1);

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::vector<Particle> particles = {Particle{{0, 0, 0}}};

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
