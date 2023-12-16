#pragma once

#include "errors.hpp"
#include "instruction.hpp"
#include "token.hpp"
#include <magic_enum.hpp>
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

    Token const& expect(std::convertible_to<TokenType> auto... token_types) {
        static_assert(sizeof...(token_types) > 0);

        auto result = static_cast<Token const*>(nullptr);
        auto const token_found = ([&] {
            if (current().type() == token_types) {
                result = &advance();
                return true;
            }
            return false;
        }() || ...);

        auto allowed_tokens = std::string{};
        ([&] { allowed_tokens = std::format("{}{}, ", allowed_tokens, magic_enum::enum_name(token_types)); }(), ...);
        allowed_tokens.pop_back();
        allowed_tokens.pop_back();

        if (token_found) {
            return *result;
        }
        throw chissembler::EmitterError{ std::format(
                "expected token type {}, got {} instead",
                allowed_tokens,
                magic_enum::enum_name(current().type())
        ) };
    }

    [[nodiscard]] Target read_target();
    [[nodiscard]] Target write_target();
};
