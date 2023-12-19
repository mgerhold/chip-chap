#pragma once

#include "types.hpp"
#include <sstream>

template<std::integral T>
[[nodiscard]] Optional<T> to_int(std::string_view const text) {
    auto stream = std::stringstream{};
    stream << text;
    auto result = T{};
    stream >> result;
    if (not stream or not stream.eof()) {
        return none;
    }
    return result;
}

template<typename... Ts, typename... Variants>
[[nodiscard]] bool is_one_of(std::variant<Variants...> const& variant) {
    return (std::holds_alternative<Ts>(variant) || ...);
}
