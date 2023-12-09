#pragma once

#include "common/types.hpp"
#include <common/ostream_formatter.hpp>
#include <iostream>
#include <string_view>

struct SourceLocation final {
    std::string_view filename;
    std::string_view source;
    usize offset;
    usize length;

    SourceLocation(
            std::string_view const filename_,
            std::string_view const source_,
            usize const offset_,
            usize const length_
    )
        : filename{ filename_ },
          source{ source_ },
          offset{ offset_ },
          length{ length_ } { }

    [[nodiscard]] std::string_view lexeme() const {
        return source.substr(offset, length);
    }

    [[nodiscard]] std::pair<usize, usize> line_and_column() const {
        auto line = usize{ 1 };
        auto column = usize{ 1 };
        for (usize i = 0; i < offset; ++i) {
            if (source.at(i) == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }
        return { line, column };
    }

    friend std::ostream& operator<<(std::ostream& os, SourceLocation const& source_location) {
        auto const [line, column] = source_location.line_and_column();
        return os << source_location.filename << ':' << line << ':' << column;
    }
};

template<>
struct std::formatter<SourceLocation> : OStreamFormatter { };
