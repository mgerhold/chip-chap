#include <chissembler/chissembler.hpp>
#include <iostream>
#include <string_view>

int main() {
    using namespace std::string_view_literals;
    static constexpr auto source = R"(start:
one_more:
    copy 42 V0
    copy V0 V1
later:
    add 1 V1
)"sv;
    try {
        auto machine_code = chissembler::assemble("stdin", source);
    } catch (chissembler::LexerError& e) {
        std::cerr << e.what() << '\n';
    } catch (chissembler::EmitterError& e) {
        std::cerr << e.what() << '\n';
    }
}
