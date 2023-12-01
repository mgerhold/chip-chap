#pragma once

namespace emulator {

    class TimeSource {
    public:
        virtual ~TimeSource() = default;

        [[nodiscard]] virtual double elapsed_seconds() = 0;
    };

} // namespace emulator
