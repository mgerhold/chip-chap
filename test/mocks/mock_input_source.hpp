#pragma once

#include <cassert>
#include <chip8/basic_input_source.hpp>
#include <unordered_set>

class MockInputSource final : public emulator::BasicInputSource {
public:
    std::unordered_set<emulator::Key> pressed_keys;
    std::vector<emulator::Key> to_be_pressed;

    void await_keypress(std::function<void(emulator::Key)> const callback) override {
        callback(press_next());
    }

    [[nodiscard]] bool is_key_pressed(emulator::Key const key) override {
        return pressed_keys.contains(key);
    }

private:
    [[nodiscard]] emulator::Key press_next() {
        assert(not to_be_pressed.empty());
        auto const next = to_be_pressed.back();
        to_be_pressed.pop_back();
        pressed_keys.insert(next);
        return next;
    }
};
