#include "chissembler/chissembler.hpp"
#include "chissembler/errors.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <string>

[[nodiscard]] int assemble(std::uint8_t const* const data, std::size_t const size) {
    auto source = std::string{};
    source.reserve(size + 1);
    std::copy_n(data, size, std::back_inserter(source));
    try {
        auto machine_code = chissembler::assemble("stdin", source);
    } catch (chissembler::EmitterError const&) {
        // may happen
    } catch (std::exception const&) {
        return 1;
    } catch (...) {
        return 1;
    }
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* const data, size_t const size) {
    return assemble(data, size);
}
