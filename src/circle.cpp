#include "circle.h"

#include <cstddef>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

std::map<CircleParams, std::shared_ptr<CircleGeometry>> Circle::geometry_cache;
std::shared_ptr<unsigned int> Circle::SHADER_PROGRAM = nullptr;

// explanation: https://siegelord.net/circle_draw
void CircleGeometry::tessellate(float cx, float cy, float r, size_t num_segments) {
    vertices.reserve((num_segments + 1) * 3);
    indices.reserve(num_segments * 3);

    vertices.insert(vertices.end(), {cx, cy, 0.0f});

    float theta = 2.0f * glm::pi<float>() / static_cast<float>(num_segments);
    float c = cosf(theta);
    float s = sinf(theta);

    glm::vec2 pos(r, 0);  // starting position

    for (size_t ii = 0; ii < num_segments; ii++) {
        vertices.insert(vertices.end(), {pos.x + cx, pos.y + cy, 0.0f});
        // apply rotation matrix
        pos = glm::vec2(c * pos.x - s * pos.y, s * pos.x + c * pos.y);
    }

    for (size_t i = 1; i < vertices.size() / 3 - 1; i++) {
        indices.insert(indices.end(), {0, static_cast<int>(i), static_cast<int>(i + 1)});
    }
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(static_cast<int>(num_segments));
}

CircleGeometry::CircleGeometry(CircleParams params) {
    tessellate(params.cx, params.cy, params.r, params.num_segments);

    glCreateBuffers(1, &VBO);
    glNamedBufferData(VBO, static_cast<int64_t>(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    glCreateBuffers(1, &EBO);
    glNamedBufferData(EBO, static_cast<int64_t>(indices.size() * sizeof(int)), indices.data(), GL_STATIC_DRAW);

    glCreateVertexArrays(1, &VAO);
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 3 * sizeof(float));
    glVertexArrayElementBuffer(VAO, EBO);
}

CircleGeometry::~CircleGeometry() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

unsigned int CircleGeometry::get_vao() { return VAO; }
std::vector<int> CircleGeometry::get_indices() { return indices; }

Circle::Circle(float cx, float cy, float r, size_t num_segments) {
    params = CircleParams{cx, cy, r, num_segments};

    auto it = geometry_cache.find(params);
    if (it == geometry_cache.end()) {
        auto geometry = std::make_shared<CircleGeometry>(params);
        geometry_cache[params] = geometry;
    }

    if (!SHADER_PROGRAM) {
        make_shader_program();
    }

    model = glm::mat4(1.0f);
}

void Circle::draw() {
    glUseProgram(*SHADER_PROGRAM);

    glUniformMatrix4fv(glGetUniformLocation(*SHADER_PROGRAM, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(geometry_cache[params]->get_vao());
    glDrawElements(GL_TRIANGLES, static_cast<int>(geometry_cache[params]->get_indices().size()), GL_UNSIGNED_INT, 0);
}

void Circle::make_shader_program() {
    std::string vertex_shader_code = read_file("shaders/vertex_shader.glsl");
    std::string fragment_shader_code = read_file("shaders/fragment_shader.glsl");
    const char *vertex_shader_source = vertex_shader_code.c_str();
    const char *fragment_shader_source = fragment_shader_code.c_str();

    unsigned int VERTEX_SHADER = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VERTEX_SHADER, 1, &vertex_shader_source, NULL);
    glCompileShader(VERTEX_SHADER);

    GLint success;
    glGetShaderiv(VERTEX_SHADER, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLint logLength;
        glGetShaderiv(VERTEX_SHADER, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorLog(logLength);
        glGetShaderInfoLog(VERTEX_SHADER, logLength, nullptr, errorLog.data());

        std::cerr << "Shader compilation failed:\n" << errorLog.data() << std::endl;

        glDeleteShader(VERTEX_SHADER);
        // TODO(me): throw error
        //   glfwTerminate();
        //   return -1;
    }

    unsigned int FRAGMENT_SHADER = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FRAGMENT_SHADER, 1, &fragment_shader_source, NULL);
    glCompileShader(FRAGMENT_SHADER);

    glGetShaderiv(FRAGMENT_SHADER, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLint logLength;
        glGetShaderiv(FRAGMENT_SHADER, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorLog(logLength);
        glGetShaderInfoLog(FRAGMENT_SHADER, logLength, nullptr, errorLog.data());

        std::cerr << "Shader compilation failed:\n" << errorLog.data() << std::endl;

        glDeleteShader(VERTEX_SHADER);
        glDeleteShader(FRAGMENT_SHADER);
        // TODO(me): throw error
        //  glfwTerminate();
        //  return -1;
    }

    // TODO(me): cehck how this works
    SHADER_PROGRAM = std::shared_ptr<unsigned int>(new unsigned int(glCreateProgram()), [](unsigned int *ptr) {
        glDeleteProgram(*ptr);
        delete ptr;
    });
    glAttachShader(*SHADER_PROGRAM, VERTEX_SHADER);
    glAttachShader(*SHADER_PROGRAM, FRAGMENT_SHADER);
    glLinkProgram(*SHADER_PROGRAM);

    glDeleteShader(VERTEX_SHADER);
    glDeleteShader(FRAGMENT_SHADER);
}

std::string Circle::read_file(std::string file_name) {
    std::ifstream file(file_name);
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void Circle::move(glm::vec3 pos) { model = glm::translate(model, pos); }
void Circle::rotate(float deg, glm::vec3 axis) { model = glm::rotate(model, glm::radians(deg), axis); }
void Circle::scale(glm::vec3 scale) { model = glm::scale(model, scale); }
unsigned int Circle::get_shader_program() { return *SHADER_PROGRAM; }
void Circle::reset() { model = glm::mat4(1); }
