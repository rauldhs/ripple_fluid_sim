#pragma once

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/vec3.hpp>
#include <vector>

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
    int width = 800, height = 600;
    double delta_time = 1;

    unsigned int VAO, VBO, instanceVBO, instanceVelocityVBO, EBO;

    int proj_uniform_location, view_uniform_location, model_uniform_location;

    unsigned int SHADER_PROGRAM;

    glm::mat4 model = glm::mat4(1), view = glm::translate(glm::mat4(1), {0, 0, 0}),
              proj = glm::perspective(45.0f, static_cast<float>(width) / static_cast<float>(height), 1.0f, 1000.0f);

    GLFWwindow* window;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::vector<Particle> particles = {Particle{{0, 0, 0}}};

    static glm::vec3 cameraPos;
    static glm::vec3 cameraFront;
    static glm::vec3 cameraUp;

    static double last_mouse_x, last_mouse_y;
    static float yaw, pitch;

    static bool first_mouse;

    static void cursor_pos_callback(GLFWwindow* _, double x_pos, double y_pos);
    static void resize_callback(GLFWwindow* window, int width, int height);
    void process_input();

    void generate_mesh(float radius, unsigned int sectorCount, unsigned int stackCount);
    void generate_mesh(float cx, float cy, float r, size_t num_segments);

    void initialize_glfw_window_context();
    void initialize_buffers();
    void initialize_shader_program();

   public:
    float particle_radius = 4.0f;

    ParticleRenderer();
    ~ParticleRenderer();
    void update_particles(std::vector<Particle>& particles);
    void draw();
    double get_delta_time();
    bool should_close();
    void prepare_imgui();

    GLFWwindow* get_window() { return window; }
};
