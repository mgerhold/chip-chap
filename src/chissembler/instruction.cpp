#include "instruction.hpp"


#include "errors.hpp"

#include <cassert>
#include <common/types.hpp>
#include <format>

static void append_instruction(EmitterState& state, u16 const instruction) {
    state.machine_code().push_back(gsl::narrow<std::byte>(instruction >> 8));
    state.machine_code().push_back(gsl::narrow<std::byte>(instruction & 0xFF));
    state.advance_adress();
}

void instruction::Label::append(EmitterState& state) const {
    auto&& [iterator, inserted] = state.labels().insert({ m_name, state.address() });
    if (not inserted) {
        throw chissembler::EmitterError{ std::format("duplicate label name '{}'", m_name) };
    }
}

void instruction::Copy::append(EmitterState& state) const {
    assert(not std::holds_alternative<U8Immediate>(m_destination) and "cannot assign into an immediate");
    visit(
            m_source,
            [&](U8Immediate const source_immediate) {
                visit(
                        m_destination,
                        [&](U8Immediate const) { std::unreachable(); },
                        [&](DataRegister const destination_register) {
                            auto const opcode = gsl::narrow<u16>(
                                    0x6000 | (std::to_underlying(destination_register) << 8) | source_immediate.value
                            );
                            append_instruction(state, opcode);
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
                                    | (std::to_underlying(source_register) << 4)
                            );
                            append_instruction(state, opcode);
                        }
                );
            }
    );
}

void instruction::Add::append(EmitterState& state) const {
    assert(not std::holds_alternative<U8Immediate>(m_destination)
           and "cannot assign result of addition into an immediate");
    visit(
            m_source,
            [&](U8Immediate const source_immediate) {
                visit(
                        m_destination,
                        [&](U8Immediate const) { std::unreachable(); },
                        [&](DataRegister const destination_register) {
                            auto const opcode = gsl::narrow<u16>(
                                    0x7000 | (std::to_underlying(destination_register) << 8) | source_immediate.value
                            );
                            append_instruction(state, opcode);
                        }
                );
            },
            [&](DataRegister const source_register) {
                visit(
                        m_destination,
                        [&](U8Immediate const) { std::unreachable(); },
                        [&](DataRegister const destination_register) {
                            auto const opcode = gsl::narrow<u16>(
                                    0x8004 | (std::to_underlying(destination_register) << 8)
                                    | (std::to_underlying(source_register) << 4)
                            );
                            append_instruction(state, opcode);
                        }
                );
            }
    );
}
