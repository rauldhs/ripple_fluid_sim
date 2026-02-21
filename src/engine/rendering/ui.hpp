#pragma once

#include <functional>
#include <vector>

#include "engine/simulation/simulation.hpp"
#include "imgui_impl_glfw.h"

class Ui {
   private:
    std::vector<std::function<void(void)>> reset_button_listeners;

   public:
    explicit Ui(GLFWwindow* window);
    ~Ui();

    void draw(SphSimulationData& simulation_data, const std::string& optional_text = "");
    void add_reset_button_listener(std::function<void(void)> callback);
};
