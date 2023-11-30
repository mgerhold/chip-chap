#pragma once

#include "application.hpp"
#include "renderer.hpp"
#include <imgui.h>
#include <numeric>
#include <vector>

class ChipChap final : public Application {
private:
    std::vector<double> m_deltas;
    double m_delta_display_value = 0.0;

protected:
    void update([[maybe_unused]] double delta_seconds) override {
        m_deltas.push_back(delta_seconds);
        auto const sum = std::accumulate(m_deltas.cbegin(), m_deltas.cend(), 0.0);
        if (sum >= 1.0) {
            m_delta_display_value = sum / static_cast<double>(m_deltas.size());
            m_deltas.clear();
        }
    }

    void render(Renderer& renderer) const override {
        renderer.clear(Color{ 0, 144, 158, 255 }).flush();
    }

    void imgui_render() const override {
        ImGui::Begin("Stats");
        ImGui::Text("delta seconds: %f", m_delta_display_value);
        if (m_delta_display_value == 0.0) {
            ImGui::Text("fps: -");
        } else {
            ImGui::Text("fps: %f", 1.0 / m_delta_display_value);
        }
        ImGui::End();
    }
};