#pragma once

#include <array>
#include <chip8/screen.hpp>

class MockScreen final : public emulator::Screen {
    std::array<bool, 64 * 32> m_buffer = {};

public:
    bool set_pixel(u8 const x, u8 const y, bool const is_set) override {
        auto const previous = get_pixel(x, y);
        m_buffer.at(y * 64 + x) = is_set;
        return previous;
    }

    [[nodiscard]] bool get_pixel(u8 const x, u8 const y) const override {
        return m_buffer.at(y * 64 + x);
    }

    [[nodiscard]] usize width() const override {
        return 64;
    }

    [[nodiscard]] usize height() const override {
        return 32;
    }

    void clear() override {
        for (auto& value : m_buffer) {
            value = false;
        }
    }
};
