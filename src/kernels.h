#pragma once

#include "glm/ext/quaternion_geometric.hpp"
#include "glm/fwd.hpp"

// TODO: temp inlines, problably
const float EPSILON = 1e-6;

inline glm::vec3 grad_poly6(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);
    if (r_len <= 0 || r_len > h) return glm::vec3(0.0);
    const float factor = 315.0f / (64.0f * std::numbers::pi * pow(h, 9));
    return glm::vec3(static_cast<float>(-6.0 * factor * pow(h * h - r_len * r_len, 2)) * r);
}

inline float laplacian_poly6(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);
    if (r_len <= 0 || r_len > h) return 0.0;
    const float factor = 315.0 / (64.0 * std::numbers::pi * pow(h, 9));
    const float r2 = r_len * r_len;
    const float h2 = h * h;
    return -6.0 * factor * (h2 - r2) * (3.0 * h2 - 7.0 * r2);
}

inline glm::vec3 grad_spiky(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);
    if (r_len < EPSILON || r_len > h) return glm::vec3(0.0);
    if (r_len <= 0 || r_len > h) return glm::vec3(0.0);
    const float factor = 15.0 / (std::numbers::pi * pow(h, 6));
    return glm::vec3(static_cast<float>(-3.0 * factor * pow(h - r_len, 2)) * (r / (float)r_len));
}

inline float laplacian_spiky(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);
    if (r_len < EPSILON || r_len > h) return 0.0;

    if (r_len <= 0 || r_len > h) return 0.0;
    const float factor = 15.0 / (std::numbers::pi * pow(h, 6));
    return 6.0 * factor * (h - r_len) * (2.0 * r_len - h) / r_len;
}

inline glm::vec3 grad_viscosity(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);
    if (r_len < EPSILON || r_len > h) return glm::vec3(0.0);

    if (r_len <= 0 || r_len > h) return glm::vec3(0.0);
    const float factor = 15.0 / (2.0 * std::numbers::pi * pow(h, 3));
    const float dw_dr =
        factor * (-3.0 * r_len * r_len / (2.0 * pow(h, 3)) + 2.0 * r_len / (h * h) - h / (2.0 * r_len * r_len));
    return glm::vec3(dw_dr * (r / (float)r_len));
}

inline float laplacian_viscosity(const glm::vec3 &r, float h) {
    const float r_len = glm::length(r);
    if (r_len <= 0 || r_len > h) return 0.0;
    return 45.0 / (std::numbers::pi * pow(h, 5)) * (1.0 - r_len / h);
}

inline float poly6_kernel(const glm::vec3 &r, float h) {
    const float r_len = static_cast<float>(glm::length(r));
    if (0 <= r_len && r_len <= h) {
        const float factor = 315 / (64 * std::numbers::pi * pow(h, 9));
        return factor * pow((h * h - r_len * r_len), 3);
    } else {
        return 0;
    }
}

inline float spiky_kernel(const glm::vec3 &r, float h) {
    const float r_len = static_cast<float>(glm::length(r));
    if (0 <= r_len && r_len <= h) {
        const float factor = 15 / (std::numbers::pi * pow(h, 6));
        return factor * pow((h - r_len), 3);
    } else {
        return 0;
    }
}

inline float viscosity_kernel(const glm::vec3 &r, float h) {
    const float r_len = static_cast<float>(glm::length(r));

    if (r_len < EPSILON || r_len > h) return 0.0;
    if (0 <= r_len && r_len <= h) {
        const float factor = 15 / (2 * std::numbers::pi * pow(h, 3));
        return factor * (-pow(r_len, 3) / (2 * pow(h, 3)) + pow(r_len, 2) / pow(h, 2) + h / (2 * r_len) - 1);
    } else {
        return 0;
    }
}
