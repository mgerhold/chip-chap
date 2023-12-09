#pragma once

#pragma once

#include <format>
#include <sstream>

template<typename Char>
struct BasicOStreamFormatter : std::formatter<std::basic_string_view<Char>, Char> {
    template<typename T, typename OutputIt>
    OutputIt format(T const& value, std::basic_format_context<OutputIt, Char>& context) const {
        std::basic_stringstream<Char> stream;
        stream << value;
        return std::formatter<std::basic_string_view<Char>, Char>::format(stream.str(), context);
    }
};

using OStreamFormatter = BasicOStreamFormatter<char>;
