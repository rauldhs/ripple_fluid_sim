#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/core/input_manager.hpp"

struct CameraRenderData {
    glm::mat4 view = glm::translate(glm::mat4(1), {0, 0, 0}),
              proj = glm::perspective(45.0f, static_cast<float>(800) / static_cast<float>(600), 1.0f, 1000.0f);
};

class Camera {
   private:
    glm::vec3 cameraPos = glm::vec3(0, 200, 500);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    double last_mouse_x = 0, last_mouse_y = 0;
    float yaw = -90.0f, pitch = 0.0f;
    bool first_mouse = true;
    bool was_locked = true;

    float speed = 200;

    void update_look(double x_pos, double y_pos);
    void update_position(const InputState& input_state, float delta_time);

   public:
    CameraRenderData render_data;

    void update(const InputState& input_state, float delta_time);
    void set_aspect_ratio(int width, int height);
};
