#pragma once

#include "common/types.hpp"
#include "target.hpp"
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

struct AddressPlaceholder {
    u16 address;
    JumpTarget target;

    AddressPlaceholder(u16 const address, JumpTarget target) : address{ address }, target{ std::move(target) } { }
};

class EmitterState {
public:
    virtual ~EmitterState() = default;

    [[nodiscard]] virtual std::vector<std::byte>& machine_code() = 0;
    [[nodiscard]] virtual u16 address() const = 0;
    virtual void advance_adress() = 0;
    [[nodiscard]] virtual std::unordered_map<std::string, u16>& labels() = 0;
    [[nodiscard]] virtual std::vector<AddressPlaceholder>& address_placeholders() = 0;
};
