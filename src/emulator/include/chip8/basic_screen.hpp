#pragma once

#include <common/types.hpp>

namespace emulator {

    class BasicScreen {
    public:
        virtual ~BasicScreen() = default;

        virtual bool set_pixel(u8 x, u8 y, bool is_set) = 0;
        [[nodiscard]] virtual bool get_pixel(u8 x, u8 y) const = 0;
        [[nodiscard]] virtual usize width() const = 0;
        [[nodiscard]] virtual usize height() const = 0;
        virtual void clear() = 0;
    };

} // namespace emulator
