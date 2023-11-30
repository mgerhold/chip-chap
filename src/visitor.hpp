#pragma once

#include <variant>

template<typename... Ts>
struct Overloaded final : Ts... {
    using Ts::operator()...;
};

template<typename... Alternatives, typename... Visitors>
auto visit(std::variant<Alternatives...> const& variant, Visitors&&... visitors) {
    return std::visit(Overloaded<Visitors...>{ std::forward<Visitors>(visitors)... }, variant);
}
