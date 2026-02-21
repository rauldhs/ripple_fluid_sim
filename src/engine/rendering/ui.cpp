#include "engine/rendering/ui.hpp"

#include "engine/simulation/simulation.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// TODO(me): check for other alternatives for dealing with the GLFWwindow here

Ui::Ui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}
Ui::~Ui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Ui::draw(SphSimulationData& simulation_data, std::string optional_text) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Simulation Master Controls");

    if (ImGui::CollapsingHeader("Physical Constants", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Smoothing Length (h)", &simulation_data.H_SMOOTHING, 1.0f, 50.0f);
        ImGui::SliderFloat("Gas Constant", &simulation_data.GAS_CONSTANT, 0.0f, 5000.0f);
        ImGui::SliderFloat("Rest Density", &simulation_data.REST_DENSITY, 0.0f, 1000.0f);
        ImGui::SliderFloat("Viscosity", &simulation_data.VISCOSITY, 0.0f, 1000.0f);
        ImGui::SliderFloat("Gravity", &simulation_data.GRAVITY, -2000.0f, 2000.0f);
        ImGui::InputFloat("Time Step (dt)", &simulation_data.DT, 0.0001f, 0.01f, "%.4f");
        ImGui::SliderFloat("Energy loss", &simulation_data.ENERGY_LOSS, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Bounding box")) {
        ImGui::DragFloat3("Box Max Corner", &simulation_data.BOX_END.x, 1.0f, 100.0f, 2000.0f);
        ImGui::Separator();
        ImGui::SliderFloat("Ground Friction", &simulation_data.FRICTION, 0.0f, 1.0f);
    }

    ImGui::Separator();
    if (!optional_text.empty()) {
        ImGui::Text("%s", optional_text.c_str());
        ImGui::Separator();
    }

    if (ImGui::Button("Reset Simulation")) {
        for (auto& call_back : reset_button_listeners) {
            call_back();
        }
    }

    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Ui::add_reset_button_listener(std::function<void(void)> callback) {
    reset_button_listeners.push_back(std::move(callback));
}
