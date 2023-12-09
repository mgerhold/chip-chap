#pragma once

#include "target.hpp"
#include <cassert>
#include <common/types.hpp>
#include <common/visitor.hpp>
#include <gsl/gsl>
#include <memory>
#include <vector>

namespace instruction {
    class BasicInstruction;
}

using Instruction = std::unique_ptr<instruction::BasicInstruction>;

static void append_instruction(std::vector<std::byte>& machine_code, u16 const instruction) {
    machine_code.push_back(gsl::narrow<std::byte>(instruction >> 8));
    machine_code.push_back(gsl::narrow<std::byte>(instruction & 0xFF));
}

namespace instruction {

    class BasicInstruction {
    public:
        virtual ~BasicInstruction() = default;

        virtual void append(std::vector<std::byte>& machine_code) const = 0;
    };

    class Copy final : public BasicInstruction {
    private:
        Target m_source;
        Target m_destination;

    public:
        Copy(Target const& source, Target const& destination)
            : m_source{ source },
              m_destination{ destination } { }

        void append(std::vector<std::byte>& machine_code) const override {
            assert(not std::holds_alternative<U8Immediate>(m_destination) and "cannot assign into an immediate");
            visit(
                    m_source,
                    [&](U8Immediate const source_immediate) {
                        visit(
                                m_destination,
                                [&](U8Immediate const) { std::unreachable(); },
                                [&](DataRegister const destination_register) {
                                    auto const opcode = gsl::narrow<u16>(
                                            0x6000 | (std::to_underlying(destination_register) << 8)
                                            | source_immediate.value
                                    );
                                    append_instruction(machine_code, opcode);
                                }
                        );
                    },
                    [&](DataRegister const source_register) {
                        visit(
                                m_destination,
                                [&](U8Immediate const) { std::unreachable(); },
                                [&](DataRegister const destination_register) {
                                    auto const opcode = gsl::narrow<u16>(
                                            0x8000 | (std::to_underlying(destination_register) << 8)
                                            | std::to_underlying(source_register)
                                    );
                                    append_instruction(machine_code, opcode);
                                }
                        );
                    }
            );
        }
    };

} // namespace instruction
