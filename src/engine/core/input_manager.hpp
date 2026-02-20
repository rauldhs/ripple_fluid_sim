#pragma once

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <array>

struct InputState {
    double look_x, look_y;
    bool lock_mouse;
    bool unlock_mouse;

    bool move_left;
    bool move_right;
    bool move_forward;
    bool move_backward;

    bool close_app;
};

class InputManager {
   public:
    InputState input_state;

    void update_mouse_pos(double new_x, double new_y);
    void process_input();
    void update_key(int key, int action);
};
