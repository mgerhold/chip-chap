#pragma once

#include "key_code.hpp"
#include <common/types.hpp>
#include <variant>

namespace event {
    struct Quit final { };

    struct KeyDown final {
        KeyCode which;
    };

    struct KeyUp final {
        KeyCode which;
    };

    using Event = std::variant<Quit, KeyDown, KeyUp>;
} // namespace event
