#pragma once

#include <algorithm>
#include <array>

[[nodiscard]] inline bool is_valid_register_char(char const c) {
    static constexpr auto valid = std::array{
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    };
    return std::find(valid.cbegin(), valid.cend(), c) != valid.cend();
}
