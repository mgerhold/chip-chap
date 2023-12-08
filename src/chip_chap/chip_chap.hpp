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
    enum class State {
        Paused,
        Playing,
    };

    std::vector<double> m_deltas;
    double m_delta_display_value = 0.0;
    GLuint m_texture_name = 0;
    Screen m_screen;
    InputSource m_input_source;
    TimeSource m_time_source;
    emulator::Chip8 m_emulator;
    State m_state = State::Paused;
    double m_time_of_last_instruction;
    double m_instructions_per_second = 5.0;

public:
    ChipChap();
    ~ChipChap() override;

protected:
    void update() override;
    void handle_event(event::Event const& event) override;
    void imgui_render() override;

private:
    void render_keypad_window() const;
    void render_execution_window() const;
    void render_registers_window() const;
    void render_stats_window() const;
    void render_screen_window() const;
};
