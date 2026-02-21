// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "engine/rendering/particle_renderer.hpp"

#include <cstddef>
#include <fstream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <print>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "engine/rendering/camera.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void ParticleRenderer::initialize_buffers() {
    glCreateVertexArrays(1, &VAO);

    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &instanceVelocityVBO);
    glCreateBuffers(1, &instanceVBO);
    glCreateBuffers(1, &EBO);

    glVertexArrayElementBuffer(VAO, EBO);

    std::vector<glm::vec3> offsets(particles.size());
    std::transform(particles.begin(), particles.end(), offsets.begin(), [](auto& a) { return a.pos; });

    std::vector<glm::vec3> velocities(particles.size());
    std::transform(particles.begin(), particles.end(), velocities.begin(), [](auto& a) { return a.velocity; });

    glNamedBufferStorage(VBO, static_cast<int64_t>(vertices.size() * sizeof(float)), vertices.data(), 0);
    glNamedBufferStorage(EBO, static_cast<int64_t>(indices.size() * sizeof(unsigned int)), indices.data(), 0);
    glNamedBufferStorage(instanceVBO, static_cast<int64_t>(particles.size() * sizeof(glm::vec3)), offsets.data(),
                         GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(instanceVelocityVBO, static_cast<int64_t>(particles.size() * sizeof(glm::vec3)),
                         velocities.data(), GL_DYNAMIC_STORAGE_BIT);

    glEnableVertexArrayAttrib(VAO, 0);
    glEnableVertexArrayAttrib(VAO, 1);
    glEnableVertexArrayAttrib(VAO, 2);

    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 3 * sizeof(float));

    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 1, 1);
    glVertexArrayVertexBuffer(VAO, 1, instanceVBO, 0, 3 * sizeof(float));

    glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 2, 2);
    glVertexArrayVertexBuffer(VAO, 2, instanceVelocityVBO, 0, 3 * sizeof(float));

    glVertexArrayBindingDivisor(VAO, 1, 1);
    glVertexArrayBindingDivisor(VAO, 2, 1);
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
        glDeleteShader(VERTEX_SHADER);
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
}

ParticleRenderer::ParticleRenderer() {
    generate_mesh(radius, 36, 18);
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

void ParticleRenderer::update_particles(std::vector<Particle>& new_particles) {
    size_t old_size = particles.size();
    particles = new_particles;

    std::vector<glm::vec3> offsets(particles.size());
    std::transform(particles.begin(), particles.end(), offsets.begin(), [](auto& a) { return a.pos; });

    std::vector<glm::vec3> velocities(particles.size());
    std::transform(particles.begin(), particles.end(), velocities.begin(), [](auto& a) { return a.velocity; });

    if (new_particles.size() > old_size) {
        glDeleteBuffers(1, &instanceVBO);
        glCreateBuffers(1, &instanceVBO);
        glDeleteBuffers(1, &instanceVelocityVBO);
        glCreateBuffers(1, &instanceVelocityVBO);

        glNamedBufferStorage(instanceVBO, static_cast<int>(particles.size() * 2 * 2 * sizeof(glm::vec3)), nullptr,
                             GL_DYNAMIC_STORAGE_BIT);
        glNamedBufferStorage(instanceVelocityVBO, static_cast<int>(particles.size() * 2 * 2 * sizeof(glm::vec3)),
                             nullptr, GL_DYNAMIC_STORAGE_BIT);

        glVertexArrayVertexBuffer(VAO, 1, instanceVBO, 0, 3 * sizeof(float));
        glVertexArrayVertexBuffer(VAO, 2, instanceVelocityVBO, 0, 3 * sizeof(float));
    }

    glNamedBufferSubData(instanceVBO, 0, static_cast<int>(particles.size() * sizeof(glm::vec3)), offsets.data());
    glNamedBufferSubData(instanceVelocityVBO, 0, static_cast<int>(particles.size() * sizeof(glm::vec3)),
                         velocities.data());
}

void ParticleRenderer::draw(const CameraRenderData& camera_render_data) {
    auto start = glfwGetTime();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(SHADER_PROGRAM);

    model = glm::scale(glm::mat4(1), {radius, radius, radius});

    glProgramUniformMatrix4fv(SHADER_PROGRAM, model_uniform_location, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniformMatrix4fv(SHADER_PROGRAM, view_uniform_location, 1, GL_FALSE,
                              glm::value_ptr(camera_render_data.view));

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, 0,
                            static_cast<int>(particles.size()));

    auto end = glfwGetTime();
    static double last = 0;
    double now = glfwGetTime();
    float delta_time = static_cast<float>(end - start);
    if (now - last > 1.0) {
        std::println("rendering time: {}", delta_time);
        last = now;
    }
}

// https://songho.ca/opengl/gl_sphere.html
void ParticleRenderer::generate_mesh(float sphere_radius, unsigned int sectorCount, unsigned int stackCount) {
    vertices.clear();
    indices.clear();

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = std::numbers::pi_v<float> / 2 -
                           static_cast<float>(i) * (std::numbers::pi_v<float> / static_cast<float>(stackCount));
        float xy = sphere_radius * cosf(stackAngle);
        float z = sphere_radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle =
                static_cast<float>(j) * (2 * std::numbers::pi_v<float> / static_cast<float>(sectorCount));

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

void ParticleRenderer::update_proj_matrix(CameraRenderData camera_render_data) {
    glProgramUniformMatrix4fv(SHADER_PROGRAM, proj_uniform_location, 1, GL_FALSE,
                              glm::value_ptr(camera_render_data.proj));
}
