#pragma once

#include <chip8/basic_time_source.hpp>

class TimeSource final : public emulator::BasicTimeSource {
private:
    double m_elapsed = 0.0;

public:
    [[nodiscard]] double elapsed_seconds() const override;
    void advance(double seconds);
};
