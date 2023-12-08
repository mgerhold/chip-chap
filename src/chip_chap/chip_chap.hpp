#pragma once

#include "application.hpp"
#include "input_source.hpp"
#include "screen.hpp"
#include "time_source.hpp"
#include <chip8/chip8.hpp>
#include <glad/glad.h>
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
    ChipChap();
    ~ChipChap() override;

protected:
    void update() override;
    void handle_event(event::Event const& event) override;
    void imgui_render() const override;
};
