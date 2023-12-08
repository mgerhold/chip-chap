#include "chip_chap.hpp"
#include "imgui_internal.h"
#include <array>
#include <gsl/gsl>
#include <imgui.h>
#include <numeric>

static constexpr auto dimmed = IM_COL32(128, 128, 128, 255);
static constexpr auto white = IM_COL32(255, 255, 255, 255);

static constexpr auto default_key_bindings = std::array{
    // clang-format off
    KeyCode::X,
    KeyCode::Num1,
    KeyCode::Num2,
    KeyCode::Num3,
    KeyCode::Q,
    KeyCode::W,
    KeyCode::E,
    KeyCode::A,
    KeyCode::S,
    KeyCode::D,
    KeyCode::Y,
    KeyCode::C,
    KeyCode::Num4,
    KeyCode::R,
    KeyCode::F,
    KeyCode::V,
    // clang-format on
};

template<typename... Args>
void render_text(unsigned const color, bool const same_line, char const* const fmt, Args&&... args) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    if (same_line) {
        ImGui::SameLine();
    }
    ImGui::Text(fmt, std::forward<Args>(args)...);
    ImGui::PopStyleColor();
};

ChipChap::ChipChap() : m_input_source{ default_key_bindings }, m_emulator{ m_screen, m_input_source, m_time_source } {
    glGenTextures(1, &m_texture_name);
    glBindTexture(GL_TEXTURE_2D, m_texture_name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    auto write_instruction = [this, address = emulator::Chip8::Address{0x200}] (u16 const instruction) mutable {
        m_emulator.write(address, instruction >> 8);
        m_emulator.write(address + 1, instruction & 0xFF);
        address += 2;
    };

    write_instruction(0x6000); // 6XNN: Store number NN in register VX
    write_instruction(0xF029); // set address to glyph sprite
    write_instruction(0xD115); // draw glyph
    write_instruction(0x7001); // ++VX
    write_instruction(0xD115); // undraw glyph
    write_instruction(0x4010); // skip next instruction if VX != 0x10
    write_instruction(0x1200); // jump back to the start
    write_instruction(0x1202); // jump to 0x202
}

ChipChap::~ChipChap() {
    glDeleteTextures(1, &m_texture_name);
}
void ChipChap::update() {
    m_deltas.push_back(delta_seconds());
    auto const sum = std::accumulate(m_deltas.cbegin(), m_deltas.cend(), 0.0);
    if (sum >= 1.0) {
        m_delta_display_value = sum / static_cast<double>(m_deltas.size());
        m_deltas.clear();
        m_emulator.execute_next_instruction(); // todo: this is just for testing
    }
}
void ChipChap::handle_event(event::Event const& event) {
    m_input_source.handle_event(event);
}
void ChipChap::render_keypad_window() const {
    ImGui::Begin("Keypad");
    render_text(m_input_source.key_state(emulator::Key::Key1) ? white : dimmed, false, "1");
    render_text(m_input_source.key_state(emulator::Key::Key2) ? white : dimmed, true, "2");
    render_text(m_input_source.key_state(emulator::Key::Key3) ? white : dimmed, true, "3");
    render_text(m_input_source.key_state(emulator::Key::C) ? white : dimmed, true, "C");

    render_text(m_input_source.key_state(emulator::Key::Key4) ? white : dimmed, false, "4");
    render_text(m_input_source.key_state(emulator::Key::Key5) ? white : dimmed, true, "5");
    render_text(m_input_source.key_state(emulator::Key::Key6) ? white : dimmed, true, "6");
    render_text(m_input_source.key_state(emulator::Key::D) ? white : dimmed, true, "D");

    render_text(m_input_source.key_state(emulator::Key::Key7) ? white : dimmed, false, "7");
    render_text(m_input_source.key_state(emulator::Key::Key8) ? white : dimmed, true, "8");
    render_text(m_input_source.key_state(emulator::Key::Key9) ? white : dimmed, true, "9");
    render_text(m_input_source.key_state(emulator::Key::E) ? white : dimmed, true, "E");

    render_text(m_input_source.key_state(emulator::Key::A) ? white : dimmed, false, "A");
    render_text(m_input_source.key_state(emulator::Key::Key0) ? white : dimmed, true, "0");
    render_text(m_input_source.key_state(emulator::Key::B) ? white : dimmed, true, "B");
    render_text(m_input_source.key_state(emulator::Key::F) ? white : dimmed, true, "F");
    ImGui::End();
}
void ChipChap::render_execution_window() const {
    ImGui::Begin("Execution");
    using Address = emulator::Chip8::Address;
    render_text(white, false, "Instruction Pointer: 0x%04X", m_emulator.instruction_pointer());
    for (int line = 0; line < 3; ++line) {
        if ((line == 0 and m_emulator.instruction_pointer() < 8)
            or (line == 2 and m_emulator.instruction_pointer() >= m_emulator.memory().size() - 8)) {
            continue;
        }
        auto const start_address =
                gsl::narrow<Address>((m_emulator.instruction_pointer() & 0xFFFFFFF8) + (line - 1) * 8);
        render_text(white, false, "0x%04X:", start_address);
        for (Address offset = 0; offset < 8; ++offset) {
            auto const address = gsl::narrow<Address>(start_address + offset);
            if (address == m_emulator.instruction_pointer() or address == m_emulator.instruction_pointer() + 1) {
                render_text(white, true, "%02X", m_emulator.read(address));
            } else {
                render_text(dimmed, true, "%02X", m_emulator.read(address));
            }
        }
    }
    ImGui::End();
}
void ChipChap::render_registers_window() const {
    ImGui::Begin("Registers");
    for (u8 register_ = 0; register_ < 16; ++register_) {
        ImGui::Text("V%X: 0x%02X", register_, m_emulator.registers().at(register_));
    }
    ImGui::End();
}
void ChipChap::render_stats_window() const {
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
void ChipChap::render_screen_window() const {
    glClearColor(30.0f / 255.0f, 30.0f / 255.0f, 46.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
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
}

void ChipChap::imgui_render() const {
    render_screen_window();
    render_stats_window();
    render_registers_window();
    render_execution_window();
    render_keypad_window();
}
