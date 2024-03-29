#include "lexer.hpp"
#include "errors.hpp"
#include "utils.hpp"
#include <cctype>

[[nodiscard]] std::vector<Token> Lexer::tokenize(std::string_view const filename, std::string_view const source) {
    return Lexer{ filename, source }.tokenize_implementation(filename, source);
}

// clang-format off
[[nodiscard]] std::vector<Token> Lexer::tokenize_implementation(
    std::string_view const filename,
    std::string_view const source
) { // clang-format on
    auto tokens = std::vector<Token>{};
    while (not is_at_end()) {
        if (current() == '\n') {
            tokens.emplace_back(TokenType::Newline, SourceLocation{ filename, source, m_index, 1 });
            advance();
            continue;
        }

        if (current() == ':') {
            tokens.emplace_back(TokenType::Colon, SourceLocation{ filename, source, m_index, 1 });
            advance();
            continue;
        }

        if (current() == '+') {
            tokens.emplace_back(TokenType::Plus, SourceLocation{ filename, source, m_index, 1 });
            advance();
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(current()))) {
            advance();
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(current()))) {
            auto const start_index = m_index;
            advance();
            while (not is_at_end() and std::isdigit(static_cast<unsigned char>(current()))) {
                advance();
            }
            tokens.emplace_back(
                    TokenType::IntegerLiteral,
                    SourceLocation{ filename, source, start_index, m_index - start_index }
            );
            continue;
        }

        if (current() == 'V' and is_valid_register_char(peek())) {
            tokens.emplace_back(TokenType::Register, SourceLocation{ filename, source, m_index, 2 });
            advance();
            advance();
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(current()))) {
            auto const start_index = m_index;
            advance();
            while (not is_at_end() and (current() == '_' or std::isalnum(static_cast<unsigned char>(current())))) {
                advance();
            }
            auto const source_location = SourceLocation{ filename, source, start_index, m_index - start_index };
            auto const lexeme = source_location.lexeme();
            if (lexeme == "copy") {
                tokens.emplace_back(TokenType::Copy, source_location);
                continue;
            }
            if (lexeme == "add") {
                tokens.emplace_back(TokenType::Add, source_location);
                continue;
            }
            if (lexeme == "sub") {
                tokens.emplace_back(TokenType::Sub, source_location);
                continue;
            }
            if (lexeme == "and") {
                tokens.emplace_back(TokenType::And, source_location);
                continue;
            }
            if (lexeme == "or") {
                tokens.emplace_back(TokenType::Or, source_location);
                continue;
            }
            if (lexeme == "xor") {
                tokens.emplace_back(TokenType::Xor, source_location);
                continue;
            }
            if (lexeme == "jump") {
                tokens.emplace_back(TokenType::Jump, source_location);
                continue;
            }
            tokens.emplace_back(TokenType::Identifier, source_location);
            continue;
        }

        throw chissembler::LexerError{
            std::format("{}: source contains invalid characters", SourceLocation{ filename, source, m_index, 1 })
        };
    }
    tokens.emplace_back(TokenType::EndOfInput, SourceLocation{ filename, source, source.length() - 1, 1 });
    return tokens;
}
