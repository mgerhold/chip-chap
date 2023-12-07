#pragma once

#include <chip8/basic_time_source.hpp>

class TimeSource final : public emulator::BasicTimeSource {
public:
    [[nodiscard]] double elapsed_seconds() override;
};
