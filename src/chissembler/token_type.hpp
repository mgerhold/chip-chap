#pragma once

enum class TokenType {
    IntegerLiteral,
    Register,
    Identifier,
    Copy,
    Add,
    Sub,
    And,
    Or,
    Xor,
    Colon,
    Jump,
    Plus,
    Newline,
    EndOfInput,
};
