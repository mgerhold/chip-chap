#pragma once

#include "application.hpp"
#include "imgui_internal.h"
#include "input_source.hpp"
#include "screen.hpp"
#include "time_source.hpp"

#include <array>
#include <chip8/chip8.hpp>
#include <glad/glad.h>
#include <gsl/gsl>
#include <imgui.h>
#include <numeric>
#include <vector>

class ChipChap final : public Application {
private:
    std::vector<double> m_deltas;
    double m_delta_display_value = 0.0;
    GLuint m_texture_name = 0;
    Screen m_screen;
    InputSource m_input_source;
    TimeSource m_time_source;
    emulator::Chip8 m_emulator;

public:
    ChipChap() : m_emulator{ m_screen, m_input_source, m_time_source } {
        glGenTextures(1, &m_texture_name);
        glBindTexture(GL_TEXTURE_2D, m_texture_name);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        m_screen.set_pixel(0, 0, true);   // todo: remove (just for testing purposes)
        m_screen.set_pixel(63, 31, true); // todo: remove (just for testing purposes)
        m_emulator.write(0x200, 0x12);
        m_emulator.write(0x201, 0x0A);
        m_emulator.write(0x20A, 0x12);
        m_emulator.write(0x20B, 0x00);
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
            m_emulator.execute_next_instruction(); // todo: this is just for testing
        }
    }

    void imgui_render() const override {
        glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 46.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto const brightness0 = static_cast<u8>(static_cast<usize>(elapsed_seconds()) % 2 * 255);
        auto const brightness1 = static_cast<u8>(std::round((std::sin(elapsed_seconds()) + 1.0) * 255.0 / 2.0));
        auto const brightness2 = static_cast<u8>(std::round((std::cos(elapsed_seconds() * 0.8) + 1.0) * 255.0 / 2.0));
        auto const brightness3 = static_cast<u8>(std::round((std::sin(elapsed_seconds() * 1.3) + 1.0) * 255.0 / 2.0));
        auto const texture = std::array<u8, 16>{
            brightness0, brightness0, brightness0, 255, brightness1, 0, brightness1, 255,
            brightness2, brightness2, 0,           255, 0,           0, brightness3, 255,
        };
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                gsl::narrow<GLint>(m_screen.width()),
                gsl::narrow<GLint>(m_screen.height()),
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                m_screen.raw_data().data()
        );
        glBindTexture(GL_TEXTURE_2D, m_texture_name);

        static constexpr auto min_size = ImVec2{ 400.0f, 200.0f };
        static constexpr auto max_size = ImVec2{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        ImGui::SetNextWindowSizeConstraints(min_size, max_size);

        ImGui::Begin("Screen");
        auto const imgui_texture = ImTextureID{ reinterpret_cast<void*>(static_cast<intptr_t>(m_texture_name)) };
        auto const actual_size = ImGui::GetContentRegionAvail();
        auto const texture_height = std::min(actual_size.x / 2.0f, actual_size.y);
        auto const padding = ImGui::GetStyle().WindowPadding;
        auto const title_bar_height = ImGui::GetCurrentWindow()->TitleBarHeight();
        ImGui::SetCursorPos(ImVec2{
                (actual_size.x + padding.x * 2.0f - 2.0f * texture_height) / 2.0f,
                (actual_size.y + padding.y * 2.0f - texture_height) / 2.0f + title_bar_height,
        });
        ImGui::Image(imgui_texture, ImVec2{ 2 * texture_height, texture_height });
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

        ImGui::Begin("Registers");
        for (u8 register_ = 0; register_ < 16; ++register_) {
            ImGui::Text("V%X: 0x%02X", register_, m_emulator.registers().at(register_));
        }
        ImGui::End();

        ImGui::Begin("Execution");
        using Address = emulator::Chip8::Address;
        ImGui::Text("Instruction Pointer: 0x%04X", m_emulator.instruction_pointer());
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 128, 128, 255));
        for (int line = 0; line < 3; ++line) {
            if ((line == 0 and m_emulator.instruction_pointer() < 8)
                or (line == 2 and m_emulator.instruction_pointer() >= m_emulator.memory().size() - 8)) {
                continue;
            }
            auto const start_address =
                    gsl::narrow<Address>((m_emulator.instruction_pointer() & 0xFFFFFFF8) + (line - 1) * 8);
            ImGui::Text("0x%04X:", start_address);
            for (Address offset = 0; offset < 8; ++offset) {
                auto const address = gsl::narrow<Address>(start_address + offset);
                ImGui::SameLine();
                if (address == m_emulator.instruction_pointer()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                }
                ImGui::Text("%02X", m_emulator.read(address));
                if (address == m_emulator.instruction_pointer() + 1) {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::PopStyleColor();
        ImGui::End();
    }
};
