#include "emitter.hpp"
#include "../emulator/include/chip8/chip8.hpp"
#include "errors.hpp"
#include "utils.hpp"

#include <sstream>

[[nodiscard]] static u8 parse_u8(Token const& token) {
    auto stream = std::stringstream{};
    stream << token.lexeme();
    auto result = u16{}; // cannot be u8, because the string stream would then interpret it as a char
    stream >> result;
    if (not stream or not stream.eof() or result > std::numeric_limits<u8>::max()) {
        throw EmitterError{
            std::format("{}: '{}' is not a valid 8 bit value", token.source_location(), token.lexeme())
        };
    }
    return gsl::narrow<u8>(result);
}

[[nodiscard]] static DataRegister parse_data_register(Token const& token) {
    if (token.lexeme().length() != 2 or token.lexeme().front() != 'V'
        or not is_valid_register_char(token.lexeme().back())) {
        throw EmitterError{
            std::format("{}: '{}' does not name a valid data register", token.source_location(), token.lexeme())
        };
    }
    auto const value = token.lexeme().back();
    return static_cast<DataRegister>(std::isdigit(static_cast<unsigned char>(value)) ? value - '0' : 10 + value - 'A');
}

[[nodiscard]] std::vector<Instruction> Emitter::emit(std::span<Token const> const tokens) {
    return Emitter{ tokens }.emit_implementation();
}

[[nodiscard]] std::vector<Instruction> Emitter::emit_implementation() {
    auto instructions = std::vector<Instruction>{};
    while (not is_at_end()) {
        switch (current().type()) {
            case TokenType::Copy: {
                advance();
                auto const source = read_target();
                auto const destination = write_target();
                instructions.push_back(std::make_unique<instruction::Copy>(source, destination));
                break;
            }
            case TokenType::Add: {
                advance();
                auto const source = read_target();
                auto const destination = write_target();
                instructions.push_back(std::make_unique<instruction::Add>(source, destination));
                break;
            }
            default:
                throw EmitterError{ std::format("{}: unexpected token", current().source_location()) };
                break;
        }
    }
    return instructions;
}

[[nodiscard]] bool Emitter::is_at_end() const {
    return m_index >= m_tokens.size() or m_tokens[m_index].type() == TokenType::EndOfInput;
}

[[nodiscard]] Token const& Emitter::current() const {
    return is_at_end() ? m_tokens.back() : m_tokens[m_index];
}

[[nodiscard]] Token const& Emitter::peek(usize const offset) const {
    return (m_index + offset >= m_tokens.size() ? m_tokens.back() : m_tokens[m_index + offset]);
}

Token const& Emitter::advance() {
    auto const& result = current();
    if (not is_at_end()) {
        ++m_index;
    }
    return result;
}

[[nodiscard]] Target Emitter::read_target() {
    if (current().type() == TokenType::IntegerLiteral) {
        auto const result = U8Immediate{ parse_u8(current()) };
        advance();
        return result;
    }
    if (current().type() == TokenType::Register) {
        auto const result = parse_data_register(current());
        advance();
        return result;
    }
    throw EmitterError{
        std::format("{}: '{}' is not a valid target for reading", current().source_location(), current().lexeme())
    };
}

[[nodiscard]] Target Emitter::write_target() {
    if (current().type() == TokenType::Register) {
        auto const result = parse_data_register(current());
        advance();
        return result;
    }
    throw EmitterError{
        std::format("{}: '{}' is not a valid target for writing", current().source_location(), current().lexeme())
    };
}
