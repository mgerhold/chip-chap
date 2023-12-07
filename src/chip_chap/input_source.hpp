#pragma once

#include <chip8/basic_input_source.hpp>

class InputSource final : public emulator::BasicInputSource {
public:
    [[nodiscard]] emulator::Key await_keypress() override;
    [[nodiscard]] bool is_key_pressed(emulator::Key key) override;
};
