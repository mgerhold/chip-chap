#include <array>
#include <chip8/chip8.hpp>
#include <chissembler/chissembler.hpp>
#include <common/random.hpp>
#include <common/types.hpp>
#include <format>
#include <gsl/gsl>
#include <gtest/gtest.h>
#include <mock_input_source.hpp>
#include <mock_screen.hpp>
#include <mock_time_source.hpp>
#include <string_view>

using namespace std::string_view_literals;

#ifdef NDEBUG
#define DEBUG_LOG
#else
#define DEBUG_LOG(x)                           \
    do {                                       \
        std::cerr << #x << " = " << x << '\n'; \
    } while (false)
#endif

static constexpr auto data_registers = std::array{
    std::tuple{  u8{ 0 }, "V0"sv },
    std::tuple{  u8{ 1 }, "V1"sv },
    std::tuple{  u8{ 2 }, "V2"sv },
    std::tuple{  u8{ 3 }, "V3"sv },
    std::tuple{  u8{ 4 }, "V4"sv },
    std::tuple{  u8{ 5 }, "V5"sv },
    std::tuple{  u8{ 6 }, "V6"sv },
    std::tuple{  u8{ 7 }, "V7"sv },
    std::tuple{  u8{ 8 }, "V8"sv },
    std::tuple{  u8{ 9 }, "V9"sv },
    std::tuple{ u8{ 10 }, "VA"sv },
    std::tuple{ u8{ 11 }, "VB"sv },
    std::tuple{ u8{ 12 }, "VC"sv },
    std::tuple{ u8{ 13 }, "VD"sv },
    std::tuple{ u8{ 14 }, "VE"sv },
    std::tuple{ u8{ 15 }, "VF"sv },
};

[[nodiscard]] static std::vector<std::byte> combine_instructions(std::convertible_to<u16> auto... instructions) {
    auto result = std::vector<std::byte>{};
    result.reserve(sizeof...(instructions) * 2);
    (
            [&] {
                auto const msb = gsl::narrow<std::byte>(instructions >> 8);
                auto const lsb = gsl::narrow<std::byte>(instructions & 0xFF);
                result.push_back(msb);
                result.push_back(lsb);
            }(),
            ...
    );
    return result;
}

[[nodiscard]] static std::vector<std::byte> combine_instructions(std::span<u16 const> const instructions) {
    auto result = std::vector<std::byte>{};
    result.reserve(instructions.size() * 2);
    for (auto const instruction : instructions) {
        auto const msb = gsl::narrow<std::byte>(instruction >> 8);
        auto const lsb = gsl::narrow<std::byte>(instruction & 0xFF);
        result.push_back(msb);
        result.push_back(lsb);
    }
    return result;
}

struct EmulatorState {
    std::unique_ptr<MockScreen> screen;
    std::unique_ptr<MockInputSource> input;
    std::unique_ptr<MockTimeSource> time_source;
    emulator::Chip8 emulator;

    EmulatorState()
        : screen{ std::make_unique<MockScreen>() },
          input{ std::make_unique<MockInputSource>() },
          time_source{ std::make_unique<MockTimeSource>() },
          emulator{ *screen, *input, *time_source } { }
};

// clang-format off
[[nodiscard]] static EmulatorState execute(
    std::span<std::byte const> const machine_code,
    usize const num_instructions_to_execute
) { // clang-format on
    auto state = EmulatorState{};
    for (usize i = 0; i < machine_code.size(); ++i) {
        state.emulator.write(gsl::narrow<u16>(0x200 + i), static_cast<u8>(machine_code[i]));
    }
    for (usize i = 0; i < num_instructions_to_execute; ++i) {
        state.emulator.execute_next_instruction();
    }
    return state;
}

[[nodiscard]] static EmulatorState execute(std::span<std::byte const> const machine_code) {
    return execute(machine_code, machine_code.size() / 2);
}

TEST(ChissemblerTests, EmptySource) {
    auto const machine_code = chissembler::assemble("stdin"sv, ""sv);
    ASSERT_TRUE(machine_code.empty());
}

TEST(ChissemblerTests, CopyConstantIntoDataRegister) {
    auto random = Random{};
    for (auto&& [register_, register_name] : data_registers) {
        auto const value = random.u8_();
        auto const instruction = std::format("copy {} {}", value, register_name);
        auto const machine_code = chissembler::assemble("stdin"sv, instruction);
        ASSERT_EQ(machine_code, combine_instructions(0x6000 | (register_ << 8) | value));
        auto const state = execute(machine_code);

        ASSERT_EQ(state.emulator.registers().at(register_), value);
    }
}

TEST(ChissemblerTests, CopyingInvalidImmediateFails) {
    ASSERT_THROW(
            {
                try {
                    auto const machine_code = chissembler::assemble("stdin"sv, "copy 256 VA"sv);
                } catch (EmitterError const& e) {
                    ASSERT_STREQ(e.what(), "stdin:1:6: '256' is not a valid 8 bit value");
                    throw;
                }
            },
            EmitterError
    );
}

TEST(ChissemblerTests, CopyingIntoImmediateFails) {
    ASSERT_THROW(
            {
                try {
                    auto const machine_code = chissembler::assemble("stdin"sv, "copy 1 2"sv);
                } catch (EmitterError const& e) {
                    ASSERT_STREQ(e.what(), "stdin:1:8: '2' is not a valid target for writing");
                    throw;
                }
            },
            EmitterError
    );

    ASSERT_THROW(
            {
                try {
                    auto const machine_code = chissembler::assemble("stdin"sv, "copy VA 2"sv);
                } catch (EmitterError const& e) {
                    ASSERT_STREQ(e.what(), "stdin:1:9: '2' is not a valid target for writing");
                    throw;
                }
            },
            EmitterError
    );
}

TEST(ChissemblerTests, CopyFromOneRegisterIntoAnotherRegister) {
    auto random = Random();
    for (auto&& [source_register, source_register_name] : data_registers) {
        auto const value = random.u8_();
        auto source = std::string{};
        auto instructions = std::vector<u16>{};
        // store random value in source register
        source += std::format("copy {} {}\n", value, source_register_name);
        instructions.push_back(gsl::narrow<u16>(0x6000 | (source_register << 8) | value));
        for (auto&& [destination_register, destination_register_name] : data_registers) {
            // copy from source register to target register
            instructions.push_back(gsl::narrow<u16>(0x8000 | (destination_register << 8) | (source_register << 4)));
            source += std::format("copy {} {}\n", source_register_name, destination_register_name);
        }
        auto const machine_code = chissembler::assemble("stdin", source);
        ASSERT_EQ(machine_code, combine_instructions(instructions));

        auto const state = execute(machine_code);
        for (auto&& [register_, name] : data_registers) {
            ASSERT_EQ(state.emulator.registers().at(register_), value);
        }
    }
}

TEST(ChissemblerTests, AddImmediateToRegister) {
    auto random = Random{};
    for (auto&& [register_, register_name] : data_registers) {
        auto const lhs = random.u8_();
        auto const rhs = random.u8_();
        auto const source = std::format(
                R"(copy {} {}
add {} {}
)",
                lhs,
                register_name,
                rhs,
                register_name
        );
        auto const machine_code = chissembler::assemble("stdin", source);
        ASSERT_EQ(
                machine_code,
                combine_instructions(
                        gsl::narrow<u16>(0x6000 | (register_ << 8) | lhs),
                        gsl::narrow<u16>(0x7000 | (register_ << 8) | rhs)
                )
        );

        auto const state = execute(machine_code);
        ASSERT_EQ(state.emulator.registers().at(register_), gsl::narrow_cast<u8>(lhs + rhs));
    }
}

TEST(ChissemblerTests, AddRegisterToRegister) {
    auto random = Random{};
    for (auto&& [source_register, source_register_name] : data_registers) {
        // summing changed the value of VF, therefore we skip operating on that register
        if (source_register == 0xF) {
            continue;
        }
        for (auto&& [destination_register, destination_register_name] : data_registers) {
            if (destination_register == 0xF) {
                continue;
            }
            auto const lhs = random.u8_();
            auto const rhs = random.u8_();
            auto const source = std::format(
                    R"(copy {} {}
copy{} {}
add {} {}
)",
                    lhs,
                    source_register_name,
                    rhs,
                    destination_register_name,
                    source_register_name,
                    destination_register_name
            );

            auto const machine_code = chissembler::assemble("stdin", source);
            ASSERT_EQ(
                    machine_code,
                    combine_instructions(
                            gsl::narrow<u16>(0x6000 | (source_register << 8) | lhs),
                            gsl::narrow<u16>(0x6000 | (destination_register << 8) | rhs),
                            gsl::narrow<u16>(0x8004 | (destination_register << 8) | (source_register << 4))
                    )
            );

            DEBUG_LOG(source_register_name);
            DEBUG_LOG(destination_register_name);
            DEBUG_LOG(static_cast<int>(lhs));
            DEBUG_LOG(static_cast<int>(rhs));
            DEBUG_LOG(static_cast<int>(static_cast<u8>(lhs + rhs)));

            auto const state = execute(machine_code);

            auto const sum = (source_register == destination_register ? 2 * rhs : lhs + rhs);
            auto const carry = (sum > static_cast<int>(std::numeric_limits<u8>::max()));

            if (source_register != destination_register) {
                ASSERT_EQ(state.emulator.registers().at(source_register), lhs);
            }
            ASSERT_EQ(state.emulator.registers().at(destination_register), gsl::narrow_cast<u8>(sum));
            ASSERT_EQ(state.emulator.registers().at(0xF), static_cast<u8>(carry));
        }
    }
}
