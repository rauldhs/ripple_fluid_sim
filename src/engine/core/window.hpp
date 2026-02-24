#pragma once

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <functional>
#include <string>
#include <vector>

#include "engine/core/input_manager.hpp"

class Window {
   private:
    GLFWwindow* window;
    int width, height;
    std::string name;

    // TODO(me): make this more generic
    std::vector<std::function<void(int, int)>> resize_listeners;
    std::vector<std::function<void(double, double)>> cursor_pos_listeners;
    std::vector<std::function<void(int, int)>> key_action_listeners;

    static void resize_callback(GLFWwindow* window, int width, int height);
    static void cursor_pos_callback(GLFWwindow* window, double new_x, double new_y);
    static void key_callback(GLFWwindow* window, int key, int, int action, int);

   public:
    Window(int height, int width, std::string name);
    Window();
    ~Window();

    void add_resize_listener(std::function<void(int width, int height)> callback);
    void add_cursor_pos_listener(std::function<void(double, double)> callback);
    void add_key_action_listener(std::function<void(int, int)> callback);

    bool should_close();
    void update();
    void update(const InputState& input_state);
    GLFWwindow* get_handle();
};
