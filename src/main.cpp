#include "chip_chap.hpp"
#include <cstdlib>

int main(int, char**) {
    auto chip_chap = ChipChap{};
    chip_chap.run();
    return EXIT_SUCCESS;
}
