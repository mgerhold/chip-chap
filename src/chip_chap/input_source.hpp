#pragma once

#include "event.hpp"


#include "key_code.hpp"
#include <array>
#include <chip8/basic_input_source.hpp>

class InputSource final : public emulator::BasicInputSource {
private:
    std::array<KeyCode, 16> m_key_mapping;
    std::array<bool, 16> m_pressed_keys{};
    std::function<void(emulator::Key)> m_blocking_input_callback;

public:
    explicit InputSource(std::array<KeyCode, 16> const& key_mapping);

    void await_keypress(std::function<void(emulator::Key)> callback) override;
    [[nodiscard]] bool is_key_pressed(emulator::Key key) override;
    void handle_event(event::Event const& event);
    [[nodiscard]] bool is_blocked() const;
    [[nodiscard]] bool key_state(emulator::Key key) const;

private:
    [[nodiscard]] Optional<u8> key_code_to_value(KeyCode key_code) const;
};
