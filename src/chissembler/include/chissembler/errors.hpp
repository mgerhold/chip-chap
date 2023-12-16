#pragma once

#include <stdexcept>

namespace chissembler {
    class LexerError final : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class EmitterError final : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
} // namespace chissembler
