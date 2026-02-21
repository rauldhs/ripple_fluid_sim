#pragma once
#include <glm/vec3.hpp>

struct Particle {
    glm::vec3 pos;
    glm::vec3 velocity = {0, 0, 0};
    glm::vec3 accerelation = {0, 0, 0};
    float mass = 10.0f;
    float pressure = 0.0f;
    float density = 0.0f;
};
