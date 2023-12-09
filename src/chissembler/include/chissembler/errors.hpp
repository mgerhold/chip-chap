#pragma once

#include <stdexcept>

class EmitterError final : public std::runtime_error {
    using std::runtime_error::runtime_error;
};
