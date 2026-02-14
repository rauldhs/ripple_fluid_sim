// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <cstddef>
#include <glm/glm.hpp>
#include <iostream>
#include <print>
#include <random>
#include <vector>

#include "circle.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

struct CircleProperties {
    glm::vec3 position;
    glm::vec3 rotation_axis;
    float rotation_speed;
};

int WIDTH = 800, HEIGHT = 600;

void resize_callback(GLFWwindow *_, int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    glViewport(0, 0, WIDTH, HEIGHT);
}

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "failed to initialize glfw";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Circle Stress Test", NULL, NULL);
    if (!window) {
        std::cerr << "failed to create glfw window";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "failed to load glad opengl stuff";
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, WIDTH, HEIGHT);

    Circle circle;
    unsigned int SHADER_PROGRAM = circle.get_shader_program();

    glm::mat4 view(1);
    view = glm::translate(view, {0, 0, -3});

    // Random generation setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> rot_speed_dist(10.0f, 100.0f);
    std::uniform_real_distribution<float> axis_dist(-1.0f, 1.0f);

    std::vector<CircleProperties> circle_props;
    std::vector<Circle> circles;

    const size_t NUM_CIRCLES = 100000;
    std::println("Creating {} circles...", NUM_CIRCLES);

    for (size_t i = 0; i < NUM_CIRCLES; i++) {
        Circle circle_i;

        CircleProperties props;
        props.position = glm::vec3(pos_dist(gen), pos_dist(gen), pos_dist(gen));
        props.rotation_axis = glm::normalize(glm::vec3(axis_dist(gen), axis_dist(gen), axis_dist(gen)));
        props.rotation_speed = rot_speed_dist(gen);

        circle_props.push_back(props);
        circles.push_back(circle_i);
    }

    std::println("Starting render loop...");

    double delta_time = 0.016;  // Initial value to avoid division by zero
    while (!glfwWindowShouldClose(window)) {
        auto start_time = glfwGetTime();

        std::println("FPS: {:.1f}", 1.0 / delta_time);

        process_input(window);
        glClear(GL_COLOR_BUFFER_BIT);

        float time = (float)glfwGetTime();
        glm::mat4 proj = glm::perspective(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

        glUseProgram(SHADER_PROGRAM);
        glUniformMatrix4fv(glGetUniformLocation(SHADER_PROGRAM, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(SHADER_PROGRAM, "view"), 1, GL_FALSE, glm::value_ptr(view));

        for (size_t i = 0; i < circles.size(); i++) {
            circles[i].reset();
            circles[i].move(circle_props[i].position);
            circles[i].rotate(circle_props[i].rotation_speed * time, circle_props[i].rotation_axis);
            circles[i].scale({0.5, 0.5, 1});
            circles[i].draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        auto end_time = glfwGetTime();
        delta_time = end_time - start_time;
    }

    glfwTerminate();
    return 0;
}
