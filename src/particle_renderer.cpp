#include "particle_renderer.h"

#include <cstddef>
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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoaderLoadGL()) {
        throw std::runtime_error("failed to load opengl bindings");
    }
    glViewport(0, 0, width, height);
}

void ParticleRenderer::resize_callback(GLFWwindow* window, int width_new, int height_new) {
    auto* self = static_cast<ParticleRenderer*>(glfwGetWindowUserPointer(window));

    self->width = width_new;
    self->height = height_new;

    self->proj = glm::perspective(45.0f, static_cast<float>(width_new) / static_cast<float>(height_new), 1.0f, 1000.0f);

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

    std::vector<glm::vec3> velocities(particles.size());
    std::transform(particles.begin(), particles.end(), velocities.begin(), [](auto& a) { return a.velocity; });

    glCreateBuffers(1, &instanceVBO);
    glNamedBufferStorage(instanceVBO, static_cast<int64_t>(particles.size() * sizeof(glm::vec3)), offsets.data(),
                         GL_DYNAMIC_STORAGE_BIT);
    glCreateBuffers(1, &instanceVelocityVBO);
    glNamedBufferStorage(instanceVelocityVBO, static_cast<int64_t>(particles.size() * sizeof(glm::vec3)),
                         velocities.data(), GL_DYNAMIC_STORAGE_BIT);

    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 1, 1);
    glVertexArrayVertexBuffer(VAO, 1, instanceVBO, 0, 3 * sizeof(float));

    glEnableVertexArrayAttrib(VAO, 2);
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

    glProgramUniformMatrix4fv(SHADER_PROGRAM, proj_uniform_location, 1, GL_FALSE, glm::value_ptr(proj));
}

ParticleRenderer::ParticleRenderer() {
    initialize_glfw_window_context();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    generate_mesh(particle_radius, 36, 18);
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

glm::vec3 ParticleRenderer::cameraPos = glm::vec3(0, 200, 500);
glm::vec3 ParticleRenderer::cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 ParticleRenderer::cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

double ParticleRenderer::last_mouse_x, ParticleRenderer::last_mouse_y;
float ParticleRenderer::yaw = 0, ParticleRenderer::pitch = 0;

bool ParticleRenderer::first_mouse = true;

void ParticleRenderer::cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos) {
    if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
        return;
    }
    if (first_mouse) {
        last_mouse_x = x_pos;
        last_mouse_y = y_pos;
        first_mouse = false;
    }
    double x_offset = last_mouse_x - x_pos;
    double y_offset = y_pos - last_mouse_y;

    last_mouse_x = x_pos;
    last_mouse_y = y_pos;

    yaw += static_cast<float>(x_offset) * 0.1f;
    pitch += static_cast<float>(y_offset) * 0.1f;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void ParticleRenderer::process_input() {
    float speed = 5000 * delta_time;

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            first_mouse = true;  // <--- THIS PREVENTS THE JUMP
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraFront * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraFront * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
    }
}

void ParticleRenderer::draw() {
    auto start = glfwGetTime();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    process_input();

    glUseProgram(SHADER_PROGRAM);

    model = glm::scale(glm::mat4(1), {particle_radius, particle_radius, 1.0f});
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glProgramUniformMatrix4fv(SHADER_PROGRAM, model_uniform_location, 1, GL_FALSE, glm::value_ptr(model));
    glProgramUniformMatrix4fv(SHADER_PROGRAM, view_uniform_location, 1, GL_FALSE, glm::value_ptr(view));

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, 0,
                            static_cast<int>(particles.size()));

    prepare_imgui();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();

    auto end = glfwGetTime();
    static double last = 0;
    double now = glfwGetTime();
    delta_time = end - start;
    if (now - last > 1.0) {
        std::println("rendering time: {}", delta_time);
        last = now;
    }
}

void ParticleRenderer::prepare_imgui() {
    // TODO: temp moved here before rewrite lol
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Simulation Master Controls");

    if (ImGui::CollapsingHeader("Physical Constants", ImGuiTreeNodeFlags_DefaultOpen)) {
        //        ImGui::SliderFloat("Smoothing Length (h)", &simulation.H_SMOOTHING, 1.0f, 50.0f);
        //        ImGui::SliderFloat("Gas Constant", &simulation.GAS_CONSTANT, 0.0f, 5000.0f);
        //        ImGui::SliderFloat("Rest Density", &simulation.REST_DENSITY, 0.0f, 1000.0f);
        //        ImGui::SliderFloat("Viscosity", &simulation.VISCOSITY, 0.0f, 1000.0f);
        //        ImGui::SliderFloat("Gravity", &simulation.GRAVITY, -2000.0f, 2000.0f);
        //        ImGui::InputFloat("Time Step (dt)", &simulation.DT, 0.0001f, 0.01f, "%.4f");
    }

    if (ImGui::CollapsingHeader("Boundary &simulation. Bounding Box")) {
        //       ImGui::DragFloat3("Box Max Corner", &simulation.BOX_END.x, 1.0f, 100.0f, 2000.0f);
        //       ImGui::Separator();
        //       ImGui::SliderFloat("Bounciness", &simulation.ENERGY_LOSS, 0.0f, 1.0f);
        //       ImGui::SliderFloat("Ground Friction", &simulation.FRICTION, 0.0f, 1.0f);
    }

    ImGui::Separator();
    auto& p = particles[0];
    ImGui::Text("density %f , mass %f , velocity %f , pressure %f", p.density, p.mass, glm::length(p.velocity),
                p.pressure);
    ImGui::Separator();

    if (ImGui::Button("Reset Simulation")) {
        //      simulation.regenerate_particles(particles, particle_spacing);
    }

    ImGui::End();
}

void ParticleRenderer::generate_mesh(float radius, unsigned int sectorCount, unsigned int stackCount) {
    // TODO: check how this works actually xd
    vertices.clear();
    indices.clear();

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = std::numbers::pi / 2 - i * (std::numbers::pi / stackCount);
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * (2 * std::numbers::pi / sectorCount);

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

double ParticleRenderer::get_delta_time() { return delta_time; }
