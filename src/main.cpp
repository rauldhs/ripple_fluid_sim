// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

#include "glm/ext/scalar_constants.hpp"

std::string read_file(std::string file_name) {
    std::ifstream file(file_name);
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// explanation: https://siegelord.net/circle_draw
std::pair<std::vector<float>, std::vector<int>> get_circle_vertices(float cx, float cy, float r, int num_segments) {
    std::vector<float> circle_vertices;
    std::vector<int> circle_indices;

    circle_vertices.reserve((num_segments + 1) * 3);
    circle_indices.reserve(num_segments * 3);

    circle_vertices.insert(circle_vertices.end(), {cx, cy, 0.0f});

    float theta = 2.0f * glm::pi<float>() / static_cast<float>(num_segments);
    float c = cosf(theta);
    float s = sinf(theta);

    glm::vec2 pos(r, 0);  // starting position

    for (int ii = 0; ii < num_segments; ii++) {
        circle_vertices.insert(circle_vertices.end(), {pos.x + cx, pos.y + cy, 0.0f});
        // apply rotation matrix
        pos = glm::vec2(c * pos.x - s * pos.y, s * pos.x + c * pos.y);
    }

    for (int i = 1; i < circle_vertices.size() / 3 - 1; i++) {
        circle_indices.insert(circle_indices.end(), {0, i, i + 1});
    }
    circle_indices.push_back(0);
    circle_indices.push_back(1);
    circle_indices.push_back(num_segments);

    return {circle_vertices, circle_indices};
}
void resize_callback(GLFWwindow *_, int width, int height) { glViewport(0, 0, width, height); }

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    int WIDTH = 800, HEIGHT = 600;

    if (!glfwInit()) {
        std::cerr << "failed to initialize glfw";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "sim", NULL, NULL);
    if (!window) {
        std::cerr << "failed to create glfw window";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, resize_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "failed to load glad opengl stuff";
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, WIDTH, HEIGHT);

    auto [circle_vertices, circle_indices] = get_circle_vertices(0, 0, 1, 50);
    unsigned int VAO, VBO, EBO;

    glCreateBuffers(1, &VBO);
    glNamedBufferData(VBO, circle_vertices.size() * sizeof(float), circle_vertices.data(), GL_STATIC_DRAW);

    glCreateBuffers(1, &EBO);
    glNamedBufferData(EBO, circle_indices.size() * sizeof(int), circle_indices.data(), GL_STATIC_DRAW);

    glCreateVertexArrays(1, &VAO);
    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 3 * sizeof(float));
    glVertexArrayElementBuffer(VAO, EBO);

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
        glfwTerminate();
        return -1;
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
        glfwTerminate();
        return -1;
    }

    unsigned int SHADER_PROGRAM = glCreateProgram();
    glAttachShader(SHADER_PROGRAM, VERTEX_SHADER);
    glAttachShader(SHADER_PROGRAM, FRAGMENT_SHADER);
    glLinkProgram(SHADER_PROGRAM);

    glDeleteShader(VERTEX_SHADER);
    glDeleteShader(FRAGMENT_SHADER);

    while (!glfwWindowShouldClose(window)) {
        process_input(window);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(SHADER_PROGRAM);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, circle_indices.size(), GL_UNSIGNED_INT, 0);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glDeleteProgram(SHADER_PROGRAM);
    glfwTerminate();
}
