#pragma once

#include "application.hpp"
#include "imgui_internal.h"

#include <array>
#include <glad/glad.h>
#include <imgui.h>
#include <numeric>
#include <vector>

class ChipChap final : public Application {
private:
    std::vector<double> m_deltas;
    double m_delta_display_value = 0.0;
    GLuint m_texture_name = 0;

public:
    ChipChap() {
        glGenTextures(1, &m_texture_name);
        glBindTexture(GL_TEXTURE_2D, m_texture_name);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    ~ChipChap() override {
        glDeleteTextures(1, &m_texture_name);
    }

protected:
    void update() override {
        m_deltas.push_back(delta_seconds());
        auto const sum = std::accumulate(m_deltas.cbegin(), m_deltas.cend(), 0.0);
        if (sum >= 1.0) {
            m_delta_display_value = sum / static_cast<double>(m_deltas.size());
            m_deltas.clear();
        }
    }

    void imgui_render() const override {
        glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 46.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto const brightness0 = static_cast<u8>(static_cast<usize>(elapsed_seconds()) % 2 * 255);
        auto const brightness1 = static_cast<u8>(std::round((std::sin(elapsed_seconds()) + 1.0) * 255.0 / 2.0));
        auto const brightness2 = static_cast<u8>(std::round((std::cos(elapsed_seconds() * 0.8) + 1.0) * 255.0 / 2.0));
        auto const brightness3 = static_cast<u8>(std::round((std::sin(elapsed_seconds() * 1.3) + 1.0) * 255.0 / 2.0));
        auto const texture = std::array{
            brightness0, brightness0, brightness0, 255, brightness1, 0, brightness1, 255,
            brightness2, brightness2, 0,           255, 0,           0, brightness3, 255,
        };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
        glBindTexture(GL_TEXTURE_2D, m_texture_name);

        static constexpr auto min_size = ImVec2{ 200.0f, 200.0f };
        static constexpr auto max_size = ImVec2{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        ImGui::SetNextWindowSizeConstraints(min_size, max_size);

        ImGui::Begin("Image");
        auto const imguiTexture = ImTextureID{ reinterpret_cast<void*>(static_cast<intptr_t>(m_texture_name)) };
        auto const actual_size = ImGui::GetContentRegionAvail();
        auto const min = std::min(actual_size.x, actual_size.y);
        auto const padding = ImGui::GetStyle().WindowPadding;
        auto const title_bar_height = ImGui::GetCurrentWindow()->TitleBarHeight();
        ImGui::SetCursorPos(ImVec2{
                (actual_size.x + padding.x * 2.0f - min) / 2.0f,
                (actual_size.y + padding.y * 2.0f - min) / 2.0f + title_bar_height,
        });
        ImGui::Image(imguiTexture, ImVec2{ min, min });
        ImGui::End();

        ImGui::Begin("Stats");
        ImGui::Text("delta: %.06f s", m_delta_display_value);
        if (m_delta_display_value == 0.0) {
            ImGui::Text("fps: -");
        } else {
            ImGui::Text("fps: %.01f", 1.0 / m_delta_display_value);
        }
        ImGui::Text("elapsed time: %.03f s", elapsed_seconds());
        ImGui::End();
    }
};
