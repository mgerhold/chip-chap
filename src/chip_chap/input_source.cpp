#include "input_source.hpp"
#include <stdexcept>

[[nodiscard]] emulator::Key InputSource::await_keypress() {
    throw std::runtime_error{ "not implemented" }; // todo
}

[[nodiscard]] bool InputSource::is_key_pressed([[maybe_unused]] emulator::Key key) {
    return false; // todo
}
