#include "engine/rendering/camera.hpp"

#include "engine/core/input_manager.hpp"

void Camera::update(const InputState& input_state, float delta_time) {
    if (input_state.lock_mouse && !was_locked) {
        first_mouse = true;
    }

    was_locked = input_state.lock_mouse;

    if (input_state.lock_mouse) {
        update_look(input_state.look_x, input_state.look_y);
    }

    update_position(input_state, delta_time);

    render_data.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::set_aspect_ratio(int width, int height) {
    render_data.proj = glm::perspective(45.0f, static_cast<float>(width) / static_cast<float>(height), 1.0f, 10000.0f);
}

void Camera::update_position(const InputState& input_state, float delta_time) {
    if (input_state.move_forward) {
        cameraPos += cameraFront * speed * delta_time;
    }
    if (input_state.move_backward) {
        cameraPos -= cameraFront * speed * delta_time;
    }
    if (input_state.move_left) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed * delta_time;
    }
    if (input_state.move_right) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed * delta_time;
    }

    render_data.pos = cameraPos;
}
void Camera::update_look(double x_pos, double y_pos) {
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
    direction.x = cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    direction.y = sinf(glm::radians(pitch));
    direction.z = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}
