#pragma once

#include <chip8/time_source.hpp>

class MockTimeSource final : public emulator::TimeSource {
public:
    double elapsed = 0.0;

    MockTimeSource() = default;

    [[nodiscard]] double elapsed_seconds() override {
        return elapsed;
    }
};
