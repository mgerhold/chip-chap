#pragma once

#include <common/types.hpp>
#include <string>
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

struct V0Offset {};

using JumpTarget = std::variant<u16, std::tuple<u16, V0Offset>, std::string, std::tuple<std::string, V0Offset>>;
