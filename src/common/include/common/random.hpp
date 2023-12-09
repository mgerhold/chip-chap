#pragma once

#include "types.hpp"
#include <gsl/gsl>
#include <random>

class Random {
private:
    std::mt19937 m_generator;
    std::uniform_int_distribution<int> m_8bit_uniform_int_distribution;

public:
    Random() : m_generator{ std::random_device{}() }, m_8bit_uniform_int_distribution{ 0, 255 } { }

    [[nodiscard]] std::byte byte() {
        return gsl::narrow<std::byte>(m_8bit_uniform_int_distribution(m_generator));
    }

    [[nodiscard]] u8 u8_() {
        return static_cast<u8>(byte());
    }
};
