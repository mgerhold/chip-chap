#include "chip8.hpp"
#include <algorithm>
#include <gsl/gsl>

namespace emulator {
    Chip8::Chip8(
            BasicScreen& screen,
            BasicInputSource& input_source,
            BasicTimeSource& time_source,
            usize const memory_size
    )
        : m_random_generator{ std::random_device{}() },
          m_random_distribution{ 0, std::numeric_limits<u8>::max() },
          m_screen{ &screen },
          m_input_source{ &input_source },
          m_time_source{ &time_source },
          m_start_time{ time_source.elapsed_seconds() } {
        m_memory.resize(memory_size, u8{ 0 });

        // store default font glyphs
        static constexpr auto font_glyphs = std::array<u8, 16 * 5>{
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80, // F
        };
        std::copy(font_glyphs.cbegin(), font_glyphs.cend(), m_memory.begin());
    }

    void Chip8::execute_next_instruction() {
        if (instruction_pointer() >= m_memory.size() - 1) {
            m_halted = true;
        }

        if (m_halted) {
            return;
        }

        // clang-format off
        auto const opcode = gsl::narrow<u16>(
                (m_memory.at(instruction_pointer()) << 8)
                | m_memory.at(instruction_pointer() + 1)
            );
        // clang-format on
        auto const opcode_id = static_cast<u8>(opcode >> 12);
        auto const nn = static_cast<u8>(opcode & 0xFF);
        auto const nnn = static_cast<Address>(opcode & 0xFFF);
        auto const x = static_cast<u8>((opcode & 0xF00) >> 8);
        auto const y = static_cast<u8>((opcode & 0xF0) >> 4);
        switch (opcode_id) {
            case 0: {
                if (opcode == 0x00E0) {
                    // 00E0: Clear the screen
                    m_screen->clear();
                    break;
                }
                if (opcode == 0x00EE) {
                    // 00EE: Return from a subroutine
                    if (m_callstack.empty()) {
                        m_halted = true;
                        break;
                    }
                    auto const return_address = m_callstack.back();
                    m_callstack.pop_back();
                    m_instruction_pointer = return_address;
                    break;
                }
                m_halted = true;
                break;
            }
            case 1:
                // 1NNN: Jump to address NNN
                m_instruction_pointer = nnn;
                break;
            case 2:
                // 2NNN: Execute subroutine starting at address NNN
                m_callstack.push_back(m_instruction_pointer + 2);
                m_instruction_pointer = nnn;
                break;
            case 3:
                // 3XNN: Skip the following instruction if the value of register VX equals NN
                if (m_registers.at(x) == nn) {
                    advance();
                }
                advance();
                break;
            case 4:
                // 4XNN: Skip the following instruction if the value of register VX is not equal to NN
                if (m_registers.at(x) != nn) {
                    advance();
                }
                advance();
                break;
            case 5:
                // 5XY0: Skip the following instruction if the value of register VX is equal to the value of register VY
                if ((opcode & 0xF) != 0) {
                    m_halted = true;
                    break;
                }
                if (m_registers.at(x) == m_registers.at(y)) {
                    advance();
                }
                advance();
                break;
            case 6:
                // 6XNN: Store number NN in register VX
                m_registers.at(x) = nn;
                advance();
                break;
            case 7:
                // 7XNN: Add the value NN to register VX
                m_registers.at(x) += nn;
                advance();
                break;
            case 8:
                if ((opcode & 0xF) == 0) {
                    // 8XY0: Store the value of register VY in register VX
                    m_registers.at(x) = m_registers.at(y);
                    advance();
                    break;
                }
                if ((opcode & 0xF) == 1) {
                    // 8XY1: Set VX to VX OR VY
                    m_registers.at(x) |= m_registers.at(y);
                    advance();
                    break;
                }
                if ((opcode & 0xF) == 2) {
                    // 8XY2: Set VX to VX AND VY
                    m_registers.at(x) &= m_registers.at(y);
                    advance();
                    break;
                }
                if ((opcode & 0xF) == 3) {
                    // 8XY3: Set VX to VX XOR VY
                    m_registers.at(x) ^= m_registers.at(y);
                    advance();
                    break;
                }
                if ((opcode & 0xF) == 4) {
                    // 8XY4: Add the value of register VY to register VX
                    //       Set VF to 01 if a carry occurs
                    //       Set VF to 00 if a carry does not occur
                    auto const sum = m_registers.at(x) + m_registers.at(y);
                    auto const carry = (sum > std::numeric_limits<u8>::max());
                    m_registers.at(x) = static_cast<u8>(sum);
                    m_registers.at(0xF) = static_cast<u8>(carry);
                    advance();
                    break;
                }
                if ((opcode & 0xF) == 5) {
                    // 8XY5: Subtract the value of register VY from register VX
                    //       Set VF to 00 if a borrow occurs
                    //       Set VF to 01 if a borrow does not occur
                    auto const borrow = (m_registers.at(y) > m_registers.at(x));
                    auto const difference = static_cast<u8>(m_registers.at(x) - m_registers.at(y));
                    m_registers.at(x) = difference;
                    m_registers.at(0xF) = static_cast<u8>(not borrow);
                    advance();
                    break;
                }
                if ((opcode & 0xF) == 6) {
                    // 8XY6: Store the value of register VY shifted right one bit in register VX
                    //       Set register VF to the least significant bit prior to the shift
                    auto const lsb = static_cast<u8>(m_registers.at(y) & 0b1);
                    m_registers.at(x) = m_registers.at(y) >> 1;
                    m_registers.at(0xF) = lsb;
                    advance();
                    break;
                }
                if ((opcode & 0xF) == 7) {
                    // 8XY7: Set register VX to the value of VY minus VX
                    //       Set VF to 00 if a borrow occurs
                    //       Set VF to 01 if a borrow does not occur
                    auto const borrow = (m_registers.at(x) > m_registers.at(y));
                    auto const difference = static_cast<u8>(m_registers.at(y) - m_registers.at(x));
                    m_registers.at(x) = difference;
                    m_registers.at(0xF) = static_cast<u8>(not borrow);
                    advance();
                    break;
                }
                if ((opcode & 0xF) == 0xE) {
                    // 8XYE: Store the value of register VY shifted left one bit in register VX
                    //       Set register VF to the most significant bit prior to the shift
                    auto const msb = static_cast<u8>((m_registers.at(y) & 0b1000'0000) >> 7);
                    m_registers.at(x) = gsl::narrow<u8>(m_registers.at(y) << 1);
                    m_registers.at(0xF) = msb;
                    advance();
                    break;
                }
                m_halted = true;
                break;
            case 9:
                // 9XY0: Skip the following instruction if the value of register VX is not equal to the value of register VY
                if (m_registers.at(x) != m_registers.at(y)) {
                    advance();
                }
                advance();
                break;
            case 0xA:
                // ANNN: Store memory address NNN in register I
                m_address_register = nnn;
                advance();
                break;
            case 0xB:
                // BNNN: Jump to address NNN + V0
                m_instruction_pointer = nnn + m_registers.at(0);
                break;
            case 0xC: {
                // CXNN: Set VX to a random number with a mask of NN
                auto const random_number = static_cast<u8>(m_random_distribution(m_random_generator));
                auto const masked = static_cast<u8>(random_number & nn);
                m_registers.at(x) = masked;
                advance();
                break;
            }
            case 0xD: {
                // DXYN: Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I
                //       Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
                auto const p_x = m_registers.at(x);
                auto const p_y = m_registers.at(y);
                auto const num_rows = static_cast<u8>(opcode & 0xF);
                auto address = m_address_register;
                auto collision = false;
                for (u8 row = 0; row < num_rows; ++row, ++address) {
                    auto const row_data = read(address);
                    for (u8 column = 0; column < 8; ++column) {
                        auto const bit_mask = (1 << (7 - column));
                        auto const should_be_set = ((bit_mask & row_data) != 0);
                        auto const previous_value = m_screen->get_pixel(p_x + column, p_y + row);
                        auto const is_set = (should_be_set != previous_value);
                        m_screen->set_pixel(p_x + column, p_y + row, is_set);
                        if (previous_value and not is_set) {
                            collision = true;
                        }
                    }
                }
                m_registers.at(0xF) = static_cast<u8>(collision);
                advance();
                break;
            }
            case 0xE:
                if ((opcode & 0xFF) == 0x9E) {
                    // EX9E: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed
                    auto const key = static_cast<Key>(m_registers.at(x));
                    if (m_input_source->is_key_pressed(key)) {
                        advance();
                    }
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0xA1) {
                    // EXA1: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed
                    auto const key = static_cast<Key>(m_registers.at(x));
                    if (not m_input_source->is_key_pressed(key)) {
                        advance();
                    }
                    advance();
                    break;
                }
                m_halted = true;
                break;
            case 0xF:
                if ((opcode & 0xFF) == 0x07) {
                    // FX07: Store the current value of the delay timer in register VX
                    m_registers.at(x) = delay_timer();
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0x0A) {
                    // FX0A: Wait for a keypress and store the result in register VX
                    m_input_source->await_keypress([this, x](Key const key) {
                        m_registers.at(x) = static_cast<u8>(key);
                    });
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0x15) {
                    // FX15: Set the delay timer to the value of register VX
                    m_delay_timestamp = TimerTimestamp{ m_time_source->elapsed_seconds(), m_registers.at(x) };
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0x18) {
                    // FX18: Set the sound timer to the value of register VX
                    m_sound_timestamp = TimerTimestamp{ m_time_source->elapsed_seconds(), m_registers.at(x) };
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0x1E) {
                    // FX1E: Add the value stored in register VX to register I
                    m_address_register += m_registers.at(x);
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0x29) {
                    // FX29: Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
                    m_address_register = 5 * m_registers.at(x);
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0x33) {
                    // FX33: Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I+1, and I+2
                    auto const value = registers().at(x);
                    auto const digits = std::array<u8, 3>{
                        gsl::narrow<u8>(value / 100),
                        gsl::narrow<u8>(value / 10 % 10),
                        gsl::narrow<u8>(value % 10),
                    };
                    for (Address i = 0; i < gsl::narrow<Address>(digits.size()); ++i) {
                        write(address_register() + i, digits.at(i));
                    }
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0x55) {
                    // FX55: Store the values of registers V0 to VX inclusive in memory starting at address I
                    //       I is set to I + X + 1 after operation
                    for (u8 i = 0; i <= x; ++i) {
                        write(address_register() + i, registers().at(i));
                    }
                    m_address_register += gsl::narrow<u16>(x + 1);
                    advance();
                    break;
                }
                if ((opcode & 0xFF) == 0x65) {
                    // FX65: Fill registers V0 to VX inclusive with the values stored in memory starting at address I
                    //       I is set to I + X + 1 after operation
                    for (u8 i = 0; i <= x; ++i) {
                        m_registers.at(i) = read(address_register() + i);
                    }
                    m_address_register += gsl::narrow<u16>(x + 1);
                    advance();
                    break;
                }
                m_halted = true;
                break;
            default:
                m_halted = true;
                break;
        }
    }

    void Chip8::advance() {
        m_instruction_pointer += 2;
    }
} // namespace emulator
