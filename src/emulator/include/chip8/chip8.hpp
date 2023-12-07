#pragma once

#include "basic_screen.hpp"
#include "input_source.hpp"
#include "time_source.hpp"
#include <array>
#include <common/types.hpp>
#include <random>
#include <vector>

namespace emulator {

    class Chip8 final {
    public:
        using Address = u16;

    private:
        struct TimerTimestamp {
            double time = 0.0;
            u8 value = 0;
        };

        std::array<u8, 16> m_registers = {};
        Address m_address_register = 0;
        std::vector<u8> m_memory;
        std::vector<Address> m_callstack;
        Address m_instruction_pointer = 0x200;
        TimerTimestamp m_delay_timestamp;
        TimerTimestamp m_sound_timestamp;
        bool m_halted = false;
        std::mt19937 m_random_generator;
        std::uniform_int_distribution<int> m_random_distribution;
        BasicScreen* m_screen;
        InputSource* m_input_source;
        TimeSource* m_time_source;
        double m_start_time;

        static constexpr auto display_width = 64;
        static constexpr auto display_height = 32;

    public:
        explicit Chip8(BasicScreen& screen, InputSource& input_source, TimeSource& time_source, usize memory_size = 4 * 1024);

        void execute_next_instruction();

        [[nodiscard]] u8 read(Address const address) const {
            return m_memory.at(address);
        }

        void write(Address const address, u8 const value) {
            m_memory.at(address) = value;
        }

        [[nodiscard]] std::array<u8, 16> const& registers() const {
            return m_registers;
        }

        [[nodiscard]] Address address_register() const {
            return m_address_register;
        }

        [[nodiscard]] u8 delay_timer() const {
            auto const elapsed_seconds = m_time_source->elapsed_seconds() - m_delay_timestamp.time;
            auto const num_decrements = static_cast<usize>(elapsed_seconds * 60.0);
            if (num_decrements > static_cast<usize>(m_delay_timestamp.value)) {
                return 0;
            }
            return m_delay_timestamp.value - static_cast<u8>(num_decrements);
        }

        [[nodiscard]] u8 sound_timer() const {
            auto const elapsed_seconds = m_time_source->elapsed_seconds() - m_sound_timestamp.time;
            auto const num_decrements = static_cast<usize>(elapsed_seconds * 60.0);
            if (num_decrements > static_cast<usize>(m_sound_timestamp.value)) {
                return 0;
            }
            return m_sound_timestamp.value - static_cast<u8>(num_decrements);
        }

        [[nodiscard]] std::vector<u8> const& memory() const {
            return m_memory;
        }

        [[nodiscard]] Address instruction_pointer() const {
            return m_instruction_pointer;
        }

        [[nodiscard]] bool is_halted() const {
            return m_halted;
        }

    private:
        void advance();
    };

} // namespace emulator
