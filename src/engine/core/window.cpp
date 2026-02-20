// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
//
#include "engine/core/window.hpp"

#include <functional>
#include <memory>
#include <stdexcept>

#include "engine/core/input_manager.hpp"

bool Window::should_close() { return glfwWindowShouldClose(window); }

void Window::add_resize_listener(std::function<void(int width, int height)> callback) {
    resize_listeners.push_back(std::move(callback));
}

void Window::resize_callback(GLFWwindow* window, int new_width, int new_height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    self->width = new_width;
    self->height = new_height;

    glViewport(0, 0, self->width, self->height);
    for (auto& callback : self->resize_listeners) callback(self->width, self->height);
}

Window::Window(int height, int width, std::string name, std::shared_ptr<InputManager> input_manager = nullptr)
    : input_manager(input_manager), width(width), height(height), name(name) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
    if (!window) {
        throw std::runtime_error("failed to create glfw window");
    }
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);

    // TODO(me): this is very uggly, check better alternatives
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoaderLoadGL()) {
        throw std::runtime_error("failed to load opengl bindings");
    }
    glViewport(0, 0, width, height);
}

Window::Window() : Window(800, 600, "default window") {}

Window::~Window() {
    if (window) glfwDestroyWindow(window);
}

void Window::cursor_pos_callback(GLFWwindow* window, double new_x, double new_y) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->input_manager) {
        self->input_manager->update_mouse_pos(new_x, new_y);
    }
}

void Window::key_callback(GLFWwindow* window, int key, int, int action, int) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->input_manager) {
        self->input_manager->update_key(key, action);
    }
}

void Window::update(const InputState& input_state) {
    if (input_state.lock_mouse) {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if (input_state.unlock_mouse) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (input_state.close_app) {
        glfwSetWindowShouldClose(window, 1);
    }

    update();
}

void Window::update() {
    glfwPollEvents();
    glfwSwapBuffers(window);
}
