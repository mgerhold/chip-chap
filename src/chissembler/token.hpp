#pragma once
#include "source_location.hpp"
#include "token_type.hpp"


#include <string_view>

class Token final {
private:
    TokenType m_type;
    SourceLocation m_source_location;

public:
    Token(TokenType const type, SourceLocation const& source_location)
        : m_type{ type },
          m_source_location{ source_location } { }

    [[nodiscard]] std::string_view lexeme() const {
        return m_source_location.lexeme();
    }

    [[nodiscard]] TokenType type() const {
        return m_type;
    }

    [[nodiscard]] SourceLocation source_location() const {
        return m_source_location;
    }
};
