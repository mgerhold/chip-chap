#pragma once

#include <common/types.hpp>

namespace emulator {

    enum class Key : u8 {
        Key0 = 0x0,
        Key1 = 0x1,
        Key2 = 0x2,
        Key3 = 0x3,
        Key4 = 0x4,
        Key5 = 0x5,
        Key6 = 0x6,
        Key7 = 0x7,
        Key8 = 0x8,
        Key9 = 0x9,
        A = 0xA,
        B = 0xB,
        C = 0xC,
        D = 0xD,
        E = 0xE,
        F = 0xF,
    };

    class BasicInputSource {
    public:
        virtual ~BasicInputSource() = default;

        [[nodiscard]] virtual Key await_keypress() = 0;
        [[nodiscard]] virtual bool is_key_pressed(Key key) = 0;
    };

} // namespace emulator
