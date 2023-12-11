#pragma once

#include <chip8/basic_time_source.hpp>

class MockTimeSource final : public emulator::BasicTimeSource {
public:
    double elapsed = 0.0;

    MockTimeSource() = default;

    [[nodiscard]] double elapsed_seconds() const override {
        return elapsed;
    }
};
