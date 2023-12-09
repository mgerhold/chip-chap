#pragma once

#include "token.hpp"
#include <common/types.hpp>
#include <stdexcept>

class LexerError final : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Lexer final {
private:
    std::string_view m_filename;
    std::string_view m_source;
    usize m_index = 0;

public:
    [[nodiscard]] static std::vector<Token> tokenize(std::string_view filename, std::string_view source);

private:
    Lexer(std::string_view const filename, std::string_view const source)
        : m_filename{ filename },
          m_source{ source } { }

    [[nodiscard]] std::vector<Token> tokenize_implementation(std::string_view filename, std::string_view source);

    [[nodiscard]] bool is_at_end() const {
        return m_index >= m_source.length();
    }

    [[nodiscard]] char current() const {
        return is_at_end() ? '\0' : m_source.at(m_index);
    }

    [[nodiscard]] char peek() const {
        if (m_index + 1 >= m_source.length()) {
            return '\0';
        }
        return m_source.at(m_index + 1);
    }

    char advance() {
        auto const result = current();
        if (not is_at_end()) {
            ++m_index;
        }
        return result;
    }
};
