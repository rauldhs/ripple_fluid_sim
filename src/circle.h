#pragma once

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct CircleParams {
    float cx, cy, r;
    size_t num_segments;

    bool operator<(const CircleParams& other) const {
        if (cx != other.cx) return cx < other.cx;
        if (cy != other.cy) return cy < other.cy;
        if (r != other.r) return r < other.r;
        return num_segments < other.num_segments;
    }
};

class CircleGeometry {
   private:
    std::vector<float> vertices;
    std::vector<int> indices;
    unsigned int VAO, VBO, EBO;

    void tessellate(float cx, float cy, float r, size_t num_segments);

   public:
    unsigned int get_vao();
    std::vector<int> get_indices();

    explicit CircleGeometry(CircleParams params);
    ~CircleGeometry();
};

class Circle {
   private:
    // TODO(me): make it static
    static std::shared_ptr<unsigned int> SHADER_PROGRAM;
    static std::map<CircleParams, std::shared_ptr<CircleGeometry>> geometry_cache;

    CircleParams params;

    glm::mat4 model;

    void make_shader_program();
    std::string read_file(std::string file_name);

   public:
    Circle() : Circle(0.0f, 0.0f, 1.0f, 50) {};
    Circle(float cx, float cy, float r, size_t num_segments);

    unsigned int get_shader_program();

    void move(glm::vec3 pos);
    void rotate(float deg, glm::vec3 axis);
    void scale(glm::vec3 scale);

    void reset();

    void draw();
};
