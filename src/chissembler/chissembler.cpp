#include "chissembler.hpp"

#include "emitter.hpp"
#include "lexer.hpp"
#include "source_location.hpp"
#include <common/types.hpp>


namespace chissembler {

    [[nodiscard]] std::vector<std::byte> assemble(std::string_view const filename, std::string_view const source) {
        auto const tokens = Lexer::tokenize(filename, source);
        auto const instructions = Emitter::emit(tokens);
        auto machine_code = std::vector<std::byte>{};
        for (auto const& instruction : instructions) {
            instruction->append(machine_code);
        }
        return machine_code;
    }

} // namespace chissembler
