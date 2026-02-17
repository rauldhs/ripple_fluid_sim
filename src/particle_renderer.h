#pragma once

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/vec3.hpp>
#include <vector>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_projection.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/glm.hpp"

struct Particle {
    glm::vec3 pos;
};

class ParticleRenderer {
   private:
    int width = 800, height = 600;

    unsigned int VAO, VBO, EBO;
    unsigned int SHADER_PROGRAM;
    glm::mat4 model = glm::mat4(1), view = glm::translate(glm::mat4(1), {0, 0, -3}),
              proj = glm::perspective(45.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

    GLFWwindow* window;

    std::vector<float> vertices;
    std::vector<int> indices;

    std::vector<Particle> particles;

    static void resize_callback(GLFWwindow* window, int width, int height);
    void process_input(GLFWwindow* window);
    void generate_mesh(float cx, float cy, float r, size_t num_segments);

    void initialize_glfw_window_context();
    void initialize_buffers();
    void initialize_shader_program();

   public:
    ParticleRenderer();
    ~ParticleRenderer();
    void update_particles(std::vector<Particle>& particles);
    void draw();
    bool should_close();
};
