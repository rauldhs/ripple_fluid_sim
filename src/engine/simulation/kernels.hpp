#pragma once

#include <glm/glm.hpp>

#include "glm/fwd.hpp"

// TODO(me): temp inlines, problably
const float EPSILON = 1e-6f;

inline float poly6_kernel(const glm::vec3 &r, float h) {
    const float r_len = static_cast<float>(glm::length(r));
    if (0 <= r_len && r_len <= h) {
        const float factor = 315 / (64 * std::numbers::pi_v<float> * powf(h, 9.0f));
        return factor * powf((h * h - r_len * r_len), 3);
    } else {
        return 0;
    }
}

inline glm::vec3 grad_poly6(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);
    if (0 <= r_len && r_len <= h) {
        const float factor = 315.0f / (64.0f * std::numbers::pi_v<float> * powf(h, 9.0f));
        return static_cast<float>(-6.0f * factor * powf(h * h - r_len * r_len, 2.0f)) * r;
    } else {
        return glm::vec3(0.0);
    }
}

inline float laplacian_poly6(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);

    if (0 <= r_len && r_len <= h) {
        const float factor = 315.0f / (64.0f * std::numbers::pi_v<float> * powf(h, 9.0f));
        const float r2 = r_len * r_len;
        const float h2 = h * h;
        return -6.0f * factor * (h2 - r2) * (3.0f * h2 - 7.0f * r2);
    } else {
        return 0;
    }
}

inline float spiky_kernel(const glm::vec3 &r, float h) {
    const float r_len = static_cast<float>(glm::length(r));
    if (0 <= r_len && r_len <= h) {
        const float factor = 15 / (std::numbers::pi_v<float> * powf(h, 6));
        return factor * powf((h - r_len), 3);
    } else {
        return 0;
    }
}

inline glm::vec3 grad_spiky(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);

    if (EPSILON < r_len && r_len <= h) {
        const float factor = 15.0f / (std::numbers::pi_v<float> * powf(h, 6));
        return glm::vec3(static_cast<float>(-3.0f * factor * powf(h - r_len, 2)) * (r / r_len));
    } else {
        return glm::vec3(0.0);
    }
}

inline float laplacian_spiky(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);

    if (EPSILON < r_len && r_len <= h) {
        const float factor = 15.0f / (std::numbers::pi_v<float> * powf(h, 6.0f));
        return 6.0f * factor * (h - r_len) * (2.0f * r_len - h) / r_len;
    } else {
        return 0;
    }
}

inline float viscosity_kernel(const glm::vec3 &r, float h) {
    const float r_len = static_cast<float>(glm::length(r));

    if (EPSILON < r_len && r_len <= h) {
        const float factor = 15 / (2 * std::numbers::pi_v<float> * powf(h, 3));
        return factor * (-powf(r_len, 3) / (2 * powf(h, 3)) + powf(r_len, 2) / powf(h, 2) + h / (2 * r_len) - 1);
    } else {
        return 0;
    }
}

inline glm::vec3 grad_viscosity(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);

    if (EPSILON < r_len && r_len <= h) {
        const float factor = 15.0f / (2.0f * std::numbers::pi_v<float> * powf(h, 3));
        const float dw_dr = factor * (-3.0f * r_len * r_len / (2.0f * powf(h, 3)) + 2.0f * r_len / (h * h) -
                                      h / (2.0f * r_len * r_len));
        return glm::vec3(dw_dr * (r / r_len));
    } else {
        return glm::vec3(0);
    }
}

inline float laplacian_viscosity(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);
    if (EPSILON < r_len && r_len <= h) {
        return 45.0f / (std::numbers::pi_v<float> * powf(h, 5)) * (1.0f - r_len / h);
    } else {
        return 0;
    }
}
