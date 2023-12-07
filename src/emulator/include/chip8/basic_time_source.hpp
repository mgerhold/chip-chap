#pragma once

namespace emulator {

    class BasicTimeSource {
    public:
        virtual ~BasicTimeSource() = default;

        [[nodiscard]] virtual double elapsed_seconds() = 0;
    };

} // namespace emulator
