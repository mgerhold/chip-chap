#include "instruction.hpp"


#include "common/utils.hpp"
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
    auto&& [iterator, inserted] = state.labels().insert({ std::string{ m_label_token.lexeme() }, state.address() });
    if (not inserted) {
        throw chissembler::EmitterError{
            std::format("{}: duplicate label name '{}'", m_label_token.source_location(), m_label_token.lexeme())
        };
    }
}

void instruction::Jump::append(EmitterState& state) const {
    state.address_placeholders().emplace_back(state.address(), m_target);
    if (is_one_of<u16, std::string>(m_target)) {
        append_instruction(state, 0x1000);
    } else if (is_one_of<std::tuple<u16, V0Offset>, std::tuple<std::string, V0Offset>>(m_target)) {
        append_instruction(state, 0xB000);
    } else {
        assert(false and "invalid variant");
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

void instruction::Sub::append(EmitterState& state) const {
    assert(not std::holds_alternative<U8Immediate>(m_destination)
           and "cannot assign result of subtraction into an immediate");
    visit(
            m_source,
            [&](U8Immediate const source_immediate) {
                visit(
                        m_destination,
                        [&](U8Immediate const) { std::unreachable(); },
                        [&](DataRegister const destination_register) {
                            // there's no opcode to subtract an immediate from a
                            // register, thus we will abuse overflow here
                            auto const offset = gsl::narrow_cast<u8>(256 - source_immediate.value);
                            auto const opcode = gsl::narrow<u16>(
                                    0x7000 | (std::to_underlying(destination_register) << 8) | offset
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
                                    0x8005 | (std::to_underlying(destination_register) << 8)
                                    | (std::to_underlying(source_register) << 4)
                            );
                            append_instruction(state, opcode);
                        }
                );
            }
    );
}
