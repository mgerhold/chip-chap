#pragma once

namespace emulator {

    class BasicTimeSource {
    public:
        virtual ~BasicTimeSource() = default;

        [[nodiscard]] virtual double elapsed_seconds() const = 0;
    };

} // namespace emulator
