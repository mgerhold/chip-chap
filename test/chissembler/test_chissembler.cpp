#include <array>
#include <chissembler/chissembler.hpp>
#include <common/random.hpp>
#include <common/types.hpp>
#include <format>
#include <gsl/gsl>
#include <gtest/gtest.h>
#include <string_view>

using namespace std::string_view_literals;

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
    }
}
