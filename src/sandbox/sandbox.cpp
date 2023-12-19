#include <chissembler/chissembler.hpp>
#include <iostream>
#include <string_view>

int main() {
    using namespace std::string_view_literals;
    static constexpr auto source = R"(start:
one_more:
    copy 42 V0
start1:
    copy V0 V1
start2:
    jump start
    jump later
start3:
    jump 512
start4:
    jump start + V0
start5:
    jump 512 + V0
later:
    add 1 V1
    jump start
    jump start1
    jump start2
    jump start3
    jump start4
)"sv;
    try {
        auto machine_code = chissembler::assemble("stdin", source);
    } catch (chissembler::LexerError& e) {
        std::cerr << e.what() << '\n';
    } catch (chissembler::EmitterError& e) {
        std::cerr << e.what() << '\n';
    }
}
