#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "engine/simulation/simulation.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Ui {
   private:
    std::vector<std::function<void(void)>> reset_button_listeners;

   public:
    Ui(GLFWwindow* window);
    ~Ui();

    void draw(SphSimulationData& simulation_data, std::string optional_text = "");
    void add_reset_button_listener(std::function<void(void)> callback);
};
