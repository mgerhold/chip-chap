#pragma once

template<typename... Ts>
struct Overloaded final : Ts... {
    using Ts::operator()...;
};
