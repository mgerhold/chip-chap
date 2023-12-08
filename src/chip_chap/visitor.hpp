#pragma once

#include <variant>

template<typename... Ts>
struct Overloaded final : Ts... {
    using Ts::operator()...;
};

// clang seems to need a deduction guide (ﾉಥ益ಥ）ﾉ彡┻━┻
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

template<typename... Alternatives, typename... Visitors>
auto visit(std::variant<Alternatives...> const& variant, Visitors&&... visitors) {
    return std::visit(Overloaded<Visitors...>{ std::forward<Visitors>(visitors)... }, variant);
}
