#include "particle_renderer.h"

#include <execution>
#include <fstream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <print>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void ParticleRenderer::initialize_glfw_window_context() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "fluid_sim", NULL, NULL);
    if (!window) {
        throw std::runtime_error("failed to create glfw window");
    }
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);
    // TODO(me): this is very uggly, check better alternatives
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    if (!gladLoaderLoadGL()) {
        throw std::runtime_error("failed to load opengl bindings");
    }
    glViewport(0, 0, width, height);
}

void ParticleRenderer::resize_callback(GLFWwindow* window, int width_new, int height_new) {
    auto* self = static_cast<ParticleRenderer*>(glfwGetWindowUserPointer(window));

    self->width = width_new;
    self->height = height_new;

    self->proj =
        glm::perspective(45.0f, static_cast<float>(self->width) / static_cast<float>(self->height), 0.1f, 100.0f);

    glProgramUniformMatrix4fv(self->SHADER_PROGRAM, self->proj_uniform_location, 1, GL_FALSE,
                              glm::value_ptr(self->proj));

    glViewport(0, 0, self->width, self->height);
}

void ParticleRenderer::initialize_buffers() {
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);

    glNamedBufferStorage(VBO, static_cast<int64_t>(vertices.size() * sizeof(float)), vertices.data(), 0);
    glNamedBufferStorage(EBO, static_cast<int64_t>(indices.size() * sizeof(unsigned int)), indices.data(), 0);

    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 3 * sizeof(float));
    glVertexArrayElementBuffer(VAO, EBO);

    std::vector<glm::vec3> offsets(particles.size());
    std::transform(particles.begin(), particles.end(), offsets.begin(), [](auto& a) { return a.pos; });

    glCreateBuffers(1, &instanceVBO);
    glNamedBufferStorage(instanceVBO, static_cast<int64_t>(particles.size() * sizeof(glm::vec3)), offsets.data(),
                         GL_DYNAMIC_STORAGE_BIT);
    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 1, 1);
    glVertexArrayVertexBuffer(VAO, 1, instanceVBO, 0, 3 * sizeof(float));
    glVertexArrayBindingDivisor(VAO, 1, 1);
}

std::string read_file(std::string file_name) {
    std::ifstream file(file_name);
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
void ParticleRenderer::initialize_shader_program() {
    unsigned int VERTEX_SHADER = glCreateShader(GL_VERTEX_SHADER);
    unsigned int FRAGMENT_SHADER = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertex_shader_file = read_file("shaders/vertex_shader.glsl");
    std::string fragment_shader_file = read_file("shaders/fragment_shader.glsl");

    const char* vertex_shader_source = vertex_shader_file.c_str();
    const char* fragment_shader_source = fragment_shader_file.c_str();

    glShaderSource(VERTEX_SHADER, 1, &vertex_shader_source, 0);

    glShaderSource(FRAGMENT_SHADER, 1, &fragment_shader_source, 0);

    glCompileShader(VERTEX_SHADER);

    GLint success;
    glGetShaderiv(VERTEX_SHADER, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(VERTEX_SHADER, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> errorLog(static_cast<uint64_t>(logLength));
        glGetShaderInfoLog(VERTEX_SHADER, logLength, nullptr, errorLog.data());
        glDeleteShader(VERTEX_SHADER);
        throw std::runtime_error("failed to compile VERTEX SHADER: " + std::string(errorLog.data()));
    }
    glCompileShader(FRAGMENT_SHADER);

    glGetShaderiv(FRAGMENT_SHADER, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(FRAGMENT_SHADER, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> errorLog(static_cast<uint64_t>(logLength));
        glGetShaderInfoLog(FRAGMENT_SHADER, logLength, nullptr, errorLog.data());
        glDeleteShader(VERTEX_SHADER);  // clean up the other one
        glDeleteShader(FRAGMENT_SHADER);
        throw std::runtime_error("failed to compile FRAGMENT SHADER: " + std::string(errorLog.data()));
    }

    SHADER_PROGRAM = glCreateProgram();
    glAttachShader(SHADER_PROGRAM, VERTEX_SHADER);
    glAttachShader(SHADER_PROGRAM, FRAGMENT_SHADER);
    glLinkProgram(SHADER_PROGRAM);

    glDeleteShader(VERTEX_SHADER);
    glDeleteShader(FRAGMENT_SHADER);

    glUseProgram(SHADER_PROGRAM);

    proj_uniform_location = glGetUniformLocation(SHADER_PROGRAM, "proj");
    view_uniform_location = glGetUniformLocation(SHADER_PROGRAM, "view");
    model_uniform_location = glGetUniformLocation(SHADER_PROGRAM, "model");

    glProgramUniformMatrix4fv(SHADER_PROGRAM, proj_uniform_location, 1, GL_FALSE, glm::value_ptr(proj));
}

ParticleRenderer::ParticleRenderer() {
    initialize_glfw_window_context();
    generate_mesh(0, 0, 1, 50);
    initialize_buffers();
    initialize_shader_program();
}
ParticleRenderer::~ParticleRenderer() {
    glDeleteProgram(SHADER_PROGRAM);

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();
}

bool ParticleRenderer::should_close() { return glfwWindowShouldClose(window); }

void ParticleRenderer::update_particles(std::vector<Particle>& new_particles) {
    int old_size = particles.size();
    particles = new_particles;
    std::vector<glm::vec3> offsets(particles.size());
    std::transform(particles.begin(), particles.end(), offsets.begin(), [](auto& a) { return a.pos; });

    if (new_particles.size() > old_size) {
        glDeleteBuffers(1, &instanceVBO);
        glCreateBuffers(1, &instanceVBO);
        glNamedBufferStorage(instanceVBO, particles.size() * 2 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glVertexArrayVertexBuffer(VAO, 1, instanceVBO, 0, 3 * sizeof(float));
    }
    glNamedBufferSubData(instanceVBO, 0, particles.size() * sizeof(glm::vec3), offsets.data());
}

void ParticleRenderer::process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void ParticleRenderer::draw() {
    auto start = glfwGetTime();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    process_input(window);

    glUseProgram(SHADER_PROGRAM);

    model = glm::scale(glm::mat4(1), {0.5, 0.5, 1});

    glProgramUniformMatrix4fv(SHADER_PROGRAM, model_uniform_location, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniformMatrix4fv(SHADER_PROGRAM, view_uniform_location, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, particles.size());

    glfwSwapBuffers(window);
    glfwPollEvents();

    auto end = glfwGetTime();
    static double last = 0;
    double now = glfwGetTime();
    if (now - last > 1.0) {
        std::println("FPS: {}", 1 / (end - start));
        last = now;
    }
}

void ParticleRenderer::generate_mesh(float cx, float cy, float r, size_t num_segments) {
    vertices.reserve((num_segments + 1) * 3);
    indices.reserve(num_segments * 3);
    vertices.insert(vertices.end(), {cx, cy, 0.0f});
    float theta = 2.0f * glm::pi<float>() / static_cast<float>(num_segments);
    float c = cosf(theta);
    float s = sinf(theta);
    glm::vec2 pos(r, 0);
    for (size_t ii = 0; ii < num_segments; ii++) {
        vertices.insert(vertices.end(), {pos.x + cx, pos.y + cy, 0.0f});
        pos = glm::vec2(c * pos.x - s * pos.y, s * pos.x + c * pos.y);
    }
    for (size_t i = 1; i < vertices.size() / 3 - 1; i++) {
        indices.insert(indices.end(), {0, static_cast<unsigned int>(i), static_cast<unsigned int>(i + 1)});
    }
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(static_cast<unsigned int>(num_segments));
}
