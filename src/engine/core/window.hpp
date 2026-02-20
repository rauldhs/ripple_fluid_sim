#pragma once

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <memory>
#include <string>

#include "engine/core/input_manager.hpp"

class Window {
   private:
    GLFWwindow* window;
    std::shared_ptr<InputManager> input_manager;
    int width, height;
    std::string name;

    static void resize_callback(GLFWwindow* window, int width, int height);
    static void cursor_pos_callback(GLFWwindow* window, double new_x, double new_y);
    static void key_callback(GLFWwindow* window, int key, int, int action, int);

   public:
    Window(int height, int width, std::string name, std::shared_ptr<InputManager> input_manager);
    Window();
    ~Window();

    bool should_close();
    void update();
    void update(const InputState& input_state);
};
