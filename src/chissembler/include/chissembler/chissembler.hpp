#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace chissembler {
    [[nodiscard]] std::vector<std::byte> assemble(std::string_view filename, std::string_view source);
}
