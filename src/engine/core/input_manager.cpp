#include "engine/core/input_manager.hpp"

#include "GLFW/glfw3.h"

void InputManager::update_mouse_pos(double new_x, double new_y) {
    input_state.look_x = new_x;
    input_state.look_y = new_y;
}

void InputManager::update_key(int key, int action) {
    bool is_pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

    switch (key) {
        case GLFW_KEY_A:
            input_state.move_left = is_pressed;
            break;
        case GLFW_KEY_D:
            input_state.move_right = is_pressed;
            break;
        case GLFW_KEY_W:
            input_state.move_forward = is_pressed;
            break;
        case GLFW_KEY_S:
            input_state.move_backward = is_pressed;
            break;
        case GLFW_KEY_M:
            input_state.unlock_mouse = is_pressed;
            input_state.lock_mouse = !is_pressed;
            break;
        case GLFW_KEY_N:
            input_state.unlock_mouse = !is_pressed;
            input_state.lock_mouse = is_pressed;
            break;
        case GLFW_KEY_ESCAPE:
            input_state.close_app = is_pressed;
            break;
    }
}
