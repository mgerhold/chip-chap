#pragma once

#include <common/types.hpp>
#include <variant>

struct U8Immediate {
    u8 value;
};

enum class DataRegister : u8 {
    V0,
    V1,
    V2,
    V3,
    V4,
    V5,
    V6,
    V7,
    V8,
    V9,
    VA,
    VB,
    VC,
    VD,
    VE,
    VF,
};

using Target = std::variant<U8Immediate, DataRegister>;
