#pragma once

#include "instruction.hpp"
#include "token.hpp"
#include <span>
#include <vector>

class Emitter final {
private:
    std::span<Token const> m_tokens;
    usize m_index = 0;

    explicit Emitter(std::span<Token const> const tokens) : m_tokens{ tokens } { }

public:
    [[nodiscard]] static std::vector<Instruction> emit(std::span<Token const> tokens);

private:
    [[nodiscard]] std::vector<Instruction> emit_implementation();
    [[nodiscard]] bool is_at_end() const;
    [[nodiscard]] Token const& current() const;
    [[nodiscard]] Token const& peek(usize offset = 1) const;
    Token const& advance();
    [[nodiscard]] Target read_target();
    [[nodiscard]] Target write_target();
};
