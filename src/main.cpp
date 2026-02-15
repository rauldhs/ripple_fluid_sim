// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <cstddef>
#include <cstdlib>
#include <glm/glm.hpp>
#include <iostream>

#include "circle.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "sim", NULL, NULL);
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

    {
        Circle circle;

        glm::mat4 view(1);
        view = glm::translate(view, {0, 0, -3});
        auto SHADER_PROGRAM = circle.get_shader_program();

        while (!glfwWindowShouldClose(window)) {
            process_input(window);
            glClear(GL_COLOR_BUFFER_BIT);

            glm::mat4 proj =
                glm::perspective(45.0f, static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

            glUseProgram(*SHADER_PROGRAM);
            glUniformMatrix4fv(glGetUniformLocation(*SHADER_PROGRAM, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
            glUniformMatrix4fv(glGetUniformLocation(*SHADER_PROGRAM, "view"), 1, GL_FALSE, glm::value_ptr(view));

            circle.reset();
            circle.scale({0.3, 0.3, 1});

            circle.draw();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
