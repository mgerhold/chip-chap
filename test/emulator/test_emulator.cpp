#include "mock_input_source.hpp"
#include "mock_screen.hpp"
#include "mock_time_source.hpp"
#include <chip8/chip8.hpp>
#include <gsl/gsl>
#include <gtest/gtest.h>
#include <random>
#include <vector>

using Address = emulator::Chip8::Address;

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

class DefaultState : public ::testing::Test {
protected:
    MockScreen screen;
    MockInputSource input_source;
    MockTimeSource time_source;
    std::unique_ptr<emulator::Chip8> emulator;
    std::mt19937 random_generator{ std::random_device{}() };
    std::uniform_int_distribution<int> uniform_int_distribution;
    Address address = 0x200;

    void SetUp() override {
        time_source = MockTimeSource{};
        emulator = std::make_unique<emulator::Chip8>(screen, input_source, time_source);
        address = Address{ 0x200 };

        random_generator = std::mt19937{ std::random_device{}() };
        uniform_int_distribution = std::uniform_int_distribution<int>(0, 255); // instantiating for u8 is UB (ﾉಥ益ಥ）ﾉ彡┻━┻
    }

    [[nodiscard]] u8 random_byte() {
        return gsl::narrow<u8>(uniform_int_distribution(random_generator));
    }

    void write_opcode(u16 const opcode) {
        emulator->write(address, opcode >> 8);
        emulator->write(address + 1, opcode & 0xFF);
    }

    void write_opcode(u16 const opcode, Address const where) {
        emulator->write(where, opcode >> 8);
        emulator->write(where + 1, opcode & 0xFF);
    }

    void execute_and_advance() {
        emulator->execute_next_instruction();
        address += 2;
    }

    void execute_opcodes(std::convertible_to<u16> auto const... opcodes) {
        (
                [&] {
                    write_opcode(opcodes);
                    execute_and_advance();
                }(),
                ...
        );
    }

    void store_opcodes(std::convertible_to<u16> auto const... opcodes) {
        (
                [&] {
                    write_opcode(opcodes);
                    address += 2;
                }(),
                ...
        );
    }

    void set_register(u8 const register_, u8 const value) {
        write_set_register_opcode(register_, value, address);
        emulator->execute_next_instruction();
        address += 2;
    }

    void write_set_register_opcode(u8 const register_, u8 const value, Address const where) {
        write_opcode(0x6000 | (register_ << 8) | value, where);
    }
};

TEST_F(DefaultState, CorrectDefaultState) {
    ASSERT_EQ(emulator->registers().size(), 16);
    for (auto const register_ : emulator->registers()) {
        ASSERT_EQ(register_, 0);
    }
    ASSERT_EQ(emulator->address_register(), 0);
    ASSERT_EQ(emulator->memory().size(), 4096);
    for (Address address = 0; address < gsl::narrow<Address>(emulator->memory().size()); ++address) {
        ASSERT_EQ(emulator->read(address), address < 16 * 5 ? font_glyphs.at(address) : 0);
    }
    ASSERT_EQ(emulator->delay_timer(), 0);
    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
}

TEST_F(DefaultState, WriteAndReadMemoryExternally) {
    auto random_bytes = std::vector<u8>{};
    random_bytes.reserve(emulator->memory().size());
    for (auto i = usize{ 0 }; i < emulator->memory().size(); ++i) {
        random_bytes.push_back(random_byte());
    }

    for (auto i = usize{ 0 }; i < emulator->memory().size(); ++i) {
        emulator->write(gsl::narrow<Address>(i), random_bytes.at(i));
    }

    for (auto i = usize{ 0 }; i < emulator->memory().size(); ++i) {
        ASSERT_EQ(random_bytes.at(i), emulator->read(gsl::narrow<Address>(i)));
    }
}

// 1NNN: Jump to address NNN
TEST_F(DefaultState, Jump) {
    write_opcode(0x1300);         // jump to 0x300
    write_opcode(0x1080, 0x0300); // jump to 0x80
    write_opcode(0x1250, 0x0080); // jump to 0x250

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x300);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x80);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x250);
}

// 00E0: Clear the screen
TEST_F(DefaultState, ClearScreen) {
    screen.set_pixel(10, 10, true);

    auto any_pixel_set = false;
    for (usize y = 0; y < screen.height(); ++y) {
        for (usize x = 0; x < screen.width(); ++x) {
            if (screen.get_pixel(x, y)) {
                any_pixel_set = true;
            }
        }
    }
    ASSERT_TRUE(any_pixel_set);

    write_opcode(0x00E0, 0x200);
    emulator->execute_next_instruction();

    any_pixel_set = false;
    for (usize y = 0; y < screen.height(); ++y) {
        for (usize x = 0; x < screen.width(); ++x) {
            if (screen.get_pixel(x, y)) {
                any_pixel_set = true;
            }
        }
    }
    ASSERT_FALSE(any_pixel_set);
}

// 2NNN: Execute subroutine starting at address NNN
// 00EE: Return from a subroutine
TEST_F(DefaultState, ExecuteSubroutine) {
    write_opcode(0x2100, 0x200); // call subroutine at 0x100
    write_set_register_opcode(0xA, 42, 0x100);
    write_opcode(0x00EE, 0x102); // return from subroutine

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x100);
    ASSERT_EQ(emulator->registers()[0xA], 0);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x102);
    ASSERT_EQ(emulator->registers()[0xA], 42);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
}

// 3XNN: Skip the following instruction if the value of register VX equals NN
TEST_F(DefaultState, SkipIfRegisterEqualsConstant) {
    write_opcode(0x3A42, 0x200);                 // skip if VA equals 0x42
    write_set_register_opcode(0xB, 0x80, 0x202); // set VB to 0x80
    write_opcode(0x3B80, 0x204);                 // skip if VB equals 0x80
    write_set_register_opcode(0xB, 1, 0x206);    // set VB to 1

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
    ASSERT_EQ(emulator->registers()[0xB], 0);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);
    ASSERT_EQ(emulator->registers()[0xB], 0x80);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x208);
    ASSERT_EQ(emulator->registers()[0xB], 0x80); // unchanged
}

// 4XNN: Skip the following instruction if the value of register VX is not equal to NN
TEST_F(DefaultState, SkipIfRegisterNotEqualsConstant) {
    write_opcode(0x4A42, 0x200);                 // skip if VA not equals 0x42
    write_set_register_opcode(0xB, 0x80, 0x202); // set VB to 0x80
    write_set_register_opcode(0xB, 0x80, 0x204); // set VB to 0x80
    write_opcode(0x4B80, 0x206);                 // skip if VB not equals 0x80

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);
    ASSERT_EQ(emulator->registers()[0xB], 0); // unchanged
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x206);
    ASSERT_EQ(emulator->registers()[0xB], 0x80);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x208);
    ASSERT_EQ(emulator->registers()[0xB], 0x80); // unchanged
}

// 5XY0: Skip the following instruction if the value of register VX is equal to the value of register VY
TEST_F(DefaultState, SkipIfRegistersAreEqual) {
    write_opcode(0x5AB0, 0x200);                // skip if VA equals VB
    write_set_register_opcode(0xC, 42, 0x202);  // set VB to 42
    write_set_register_opcode(0xC, 100, 0x204); // set VB to 100
    write_set_register_opcode(0xD, 42, 0x206);  // set VD to 42
    write_opcode(0x5CD0, 0x208);                // skip if VC equals VD

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);
    ASSERT_EQ(emulator->registers()[0xC], 0); // unchanged
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x206);
    ASSERT_EQ(emulator->registers()[0xC], 100);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x208);
    ASSERT_EQ(emulator->registers()[0xD], 42);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x20A);
}

// 6XNN: Store number NN in register VX
TEST_F(DefaultState, StoreConstantInRegister) {
    for (u8 register_ = 0; register_ < 16; ++register_) {
        auto const random_number = random_byte();
        auto const opcode = 0x6000 | (register_ << 8) | random_number;
        execute_opcodes(opcode);
        ASSERT_EQ(emulator->registers().at(register_), random_number);
    }
}

// 7XNN: Add the value NN to register VX
TEST_F(DefaultState, AddConstantToRegister) {
    for (u8 register_ = 0; register_ < 16; ++register_) {
        auto const lhs = random_byte();
        auto const rhs = random_byte();
        auto const sum = static_cast<u8>(lhs + rhs);
        set_register(register_, lhs);
        execute_opcodes(
                // add rhs to register
                0x7000 | (register_ << 8) | rhs
        );
        ASSERT_EQ(emulator->registers()[register_], sum);
    }
}

// 8XY0: Store the value of register VY in register VX
TEST_F(DefaultState, CopyRegisterValue) {
    for (u8 source = 0; source < 16; ++source) {
        for (u8 destination = 0; destination < 16; ++destination) {
            auto const random_number = random_byte();
            set_register(source, random_number);
            execute_opcodes(
                    // copy from source into value
                    static_cast<u16>(0x8000 | (destination << 8) | (source << 4))
            );
            ASSERT_EQ(emulator->registers()[source], random_number);
            ASSERT_EQ(emulator->registers()[destination], random_number);
        }
    }
}

// 8XY1: Set VX to VX OR VY
TEST_F(DefaultState, OrRegisters) {
    static constexpr auto source = u8{ 7 };
    static constexpr auto destination = u8{ 10 };
    set_register(source, 0b1100'1010);
    set_register(destination, 0b0101'1011);
    execute_opcodes(0x8001 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 0b1100'1010);
    ASSERT_EQ(emulator->registers()[destination], 0b1101'1011);
}

// 8XY2: Set VX to VX AND VY
TEST_F(DefaultState, AndRegisters) {
    static constexpr auto source = u8{ 7 };
    static constexpr auto destination = u8{ 10 };
    set_register(source, 0b1100'1010);
    set_register(destination, 0b0101'1011);
    execute_opcodes(0x8002 | (destination << 8) | (source << 4));
    // 0b1100'1010
    // 0b0101'1011
    // 0b0100'1010
    ASSERT_EQ(emulator->registers()[source], 0b1100'1010);
    ASSERT_EQ(emulator->registers()[destination], 0b0100'1010);
}

// 8XY3: Set VX to VX XOR VY
TEST_F(DefaultState, XorRegisters) {
    static constexpr auto source = u8{ 7 };
    static constexpr auto destination = u8{ 10 };
    set_register(source, 0b1100'1010);
    set_register(destination, 0b0101'1011);
    execute_opcodes(0x8003 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 0b1100'1010);
    ASSERT_EQ(emulator->registers()[destination], 0b1001'0001);
}

// 8XY4: Add the value of register VY to register VX
//       Set VF to 01 if a carry occurs
//       Set VF to 00 if a carry does not occur
TEST_F(DefaultState, AddRegisters) {
    static constexpr auto source = u8{ 7 };
    static constexpr auto destination = u8{ 10 };
    set_register(source, 20);
    set_register(destination, 22);
    execute_opcodes(0x8004 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 20);
    ASSERT_EQ(emulator->registers()[destination], 42);
    ASSERT_EQ(emulator->registers()[0xF], 0); // no carry

    set_register(source, 1);
    set_register(destination, 255);
    execute_opcodes(0x8004 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 1);
    ASSERT_EQ(emulator->registers()[destination], 0);
    ASSERT_EQ(emulator->registers()[0xF], 1); // carry
}

// 8XY5: Subtract the value of register VY from register VX
//       Set VF to 00 if a borrow occurs
//       Set VF to 01 if a borrow does not occur
TEST_F(DefaultState, SubtractRegistersA) {
    static constexpr auto source = u8{ 7 };
    static constexpr auto destination = u8{ 10 };

    set_register(source, 2);
    set_register(destination, 44);
    execute_opcodes(0x8005 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 2);
    ASSERT_EQ(emulator->registers()[destination], 42);
    ASSERT_EQ(emulator->registers()[0xF], 1); // no borrow

    set_register(source, 6);
    set_register(destination, 5);
    execute_opcodes(0x8005 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 6);
    ASSERT_EQ(emulator->registers()[destination], 255);
    ASSERT_EQ(emulator->registers()[0xF], 0); // borrow
}

// 8XY6: Store the value of register VY shifted right one bit in register VX
//       Set register VF to the least significant bit prior to the shift
TEST_F(DefaultState, RightShift) {
    static constexpr auto source = u8{ 7 };
    static constexpr auto destination = u8{ 10 };

    set_register(source, 0b1100'1010);
    execute_opcodes(0x8006 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 0b1100'1010);
    ASSERT_EQ(emulator->registers()[destination], 0b0110'0101);
    ASSERT_EQ(emulator->registers()[0xF], 0);

    set_register(source, 0b1100'1011);
    execute_opcodes(0x8006 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 0b1100'1011);
    ASSERT_EQ(emulator->registers()[destination], 0b0110'0101);
    ASSERT_EQ(emulator->registers()[0xF], 1);
}

// 8XY7: Set register VX to the value of VY minus VX
//       Set VF to 00 if a borrow occurs
//       Set VF to 01 if a borrow does not occur
TEST_F(DefaultState, SubtractRegistersB) {
    static constexpr auto source = u8{ 7 };
    static constexpr auto destination = u8{ 10 };

    set_register(source, 44);
    set_register(destination, 2);
    execute_opcodes(0x8007 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 44);
    ASSERT_EQ(emulator->registers()[destination], 42);
    ASSERT_EQ(emulator->registers()[0xF], 1); // no borrow

    set_register(source, 5);
    set_register(destination, 6);
    execute_opcodes(0x8007 | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 5);
    ASSERT_EQ(emulator->registers()[destination], 255);
    ASSERT_EQ(emulator->registers()[0xF], 0); // borrow
}

// 8XYE: Store the value of register VY shifted left one bit in register VX
//       Set register VF to the most significant bit prior to the shift
TEST_F(DefaultState, LeftShift) {
    static constexpr auto source = u8{ 7 };
    static constexpr auto destination = u8{ 10 };

    set_register(source, 0b1100'1010);
    execute_opcodes(0x800E | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 0b1100'1010);
    ASSERT_EQ(emulator->registers()[destination], 0b1001'0100);
    ASSERT_EQ(emulator->registers()[0xF], 1);

    set_register(source, 0b0100'1010);
    execute_opcodes(0x800E | (destination << 8) | (source << 4));
    ASSERT_EQ(emulator->registers()[source], 0b0100'1010);
    ASSERT_EQ(emulator->registers()[destination], 0b1001'0100);
    ASSERT_EQ(emulator->registers()[0xF], 0);
}

// 9XY0: Skip the following instruction if the value of register VX is not equal to the value of register VY
TEST_F(DefaultState, SkipIfRegistersAreNotEqual) {
    write_opcode(0x9AB0, 0x200);                // skip if VA not equals VB
    write_set_register_opcode(0xC, 42, 0x202);  // set VB to 42
    write_set_register_opcode(0xC, 100, 0x204); // set VB to 100
    write_set_register_opcode(0xD, 42, 0x206);  // set VD to 42
    write_opcode(0x9CD0, 0x208);                // skip if VC not equals VD

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
    ASSERT_EQ(emulator->registers()[0xC], 0); // unchanged
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);
    ASSERT_EQ(emulator->registers()[0xC], 42);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x206);
    ASSERT_EQ(emulator->registers()[0xC], 100);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x208);
    ASSERT_EQ(emulator->registers()[0xD], 42);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x20C);
}

// ANNN: Store memory address NNN in register I
TEST_F(DefaultState, StoreIntoAddressRegister) {
    execute_opcodes(0xA123);
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
    ASSERT_EQ(emulator->address_register(), 0x123);
}

// BNNN: Jump to address NNN + V0
TEST_F(DefaultState, JumpWithOffset) {
    write_opcode(0xB300, 0x200);               // jump to 0x300
    write_set_register_opcode(0, 0x10, 0x300); // set V0 to 0x10
    write_opcode(0xB340, 0x302);               // jump to 0x340 + V0

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x300);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x302);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x350);
}

// CXNN: Set VX to a random number with a mask of NN
TEST_F(DefaultState, CreateRandomNumber) {
    static constexpr auto destination = u8{ 10 };
    // the probability of generating ten zeroes in a row is practically zero
    // and should not happen
    auto found_non_zero = false;
    for (int i = 0; i < 10; ++i) {
        execute_opcodes(0xC0FF | (destination << 8));
        if (emulator->registers()[destination] != 0) {
            found_non_zero = true;
            break;
        }
    }
    ASSERT_TRUE(found_non_zero);

    auto only_in_range = true;
    for (int i = 0; i < 10; ++i) {
        // bitmask 0xF0 = 0b0000'1111 should only allow for numbers between
        // 0b0000'0000 = 0 and 0b0000'1111 = 15.
        execute_opcodes(0xC00F | (destination << 8));
        if (emulator->registers()[destination] > 15) {
            only_in_range = false;
            break;
        }
    }
    ASSERT_TRUE(only_in_range);
}

// DXYN: Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I
//       Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
TEST_F(DefaultState, DrawSprite) {
    emulator->write(0, 0b1111'1111);
    emulator->write(1, 0b1000'0001);
    emulator->write(2, 0b1000'0001);
    emulator->write(3, 0b1000'0001);
    emulator->write(4, 0b1111'1111);
    emulator->write(5, 0b1001'1001);

    write_set_register_opcode(0, 15, 0x200); // set V0 to 15 (x)
    write_set_register_opcode(1, 8, 0x202);  // set V1 to 8 (y)
    write_opcode(0xA000, 0x204);             // set address register to 0x000
    write_opcode(0xD015, 0x206);             // draw sprite (5 rows)
    write_opcode(0xA005, 0x208);             // set address register to 0x004
    write_opcode(0xD011, 0x20A);             // draw sprite (1 row)

    emulator->execute_next_instruction();
    emulator->execute_next_instruction();
    emulator->execute_next_instruction();
    emulator->execute_next_instruction();

    ASSERT_EQ(emulator->registers()[0xF], 0x00);

    ASSERT_TRUE(screen.get_pixel(15, 8));
    ASSERT_TRUE(screen.get_pixel(16, 8));
    ASSERT_TRUE(screen.get_pixel(17, 8));
    ASSERT_TRUE(screen.get_pixel(18, 8));
    ASSERT_TRUE(screen.get_pixel(19, 8));
    ASSERT_TRUE(screen.get_pixel(20, 8));
    ASSERT_TRUE(screen.get_pixel(21, 8));
    ASSERT_TRUE(screen.get_pixel(22, 8));

    ASSERT_TRUE(screen.get_pixel(15, 9));
    ASSERT_FALSE(screen.get_pixel(16, 9));
    ASSERT_FALSE(screen.get_pixel(17, 9));
    ASSERT_FALSE(screen.get_pixel(18, 9));
    ASSERT_FALSE(screen.get_pixel(19, 9));
    ASSERT_FALSE(screen.get_pixel(20, 9));
    ASSERT_FALSE(screen.get_pixel(21, 9));
    ASSERT_TRUE(screen.get_pixel(22, 9));

    ASSERT_TRUE(screen.get_pixel(15, 10));
    ASSERT_FALSE(screen.get_pixel(16, 10));
    ASSERT_FALSE(screen.get_pixel(17, 10));
    ASSERT_FALSE(screen.get_pixel(18, 10));
    ASSERT_FALSE(screen.get_pixel(19, 10));
    ASSERT_FALSE(screen.get_pixel(20, 10));
    ASSERT_FALSE(screen.get_pixel(21, 10));
    ASSERT_TRUE(screen.get_pixel(22, 10));

    ASSERT_TRUE(screen.get_pixel(15, 11));
    ASSERT_FALSE(screen.get_pixel(16, 11));
    ASSERT_FALSE(screen.get_pixel(17, 11));
    ASSERT_FALSE(screen.get_pixel(18, 11));
    ASSERT_FALSE(screen.get_pixel(19, 11));
    ASSERT_FALSE(screen.get_pixel(20, 11));
    ASSERT_FALSE(screen.get_pixel(21, 11));
    ASSERT_TRUE(screen.get_pixel(22, 11));

    ASSERT_TRUE(screen.get_pixel(15, 12));
    ASSERT_TRUE(screen.get_pixel(16, 12));
    ASSERT_TRUE(screen.get_pixel(17, 12));
    ASSERT_TRUE(screen.get_pixel(18, 12));
    ASSERT_TRUE(screen.get_pixel(19, 12));
    ASSERT_TRUE(screen.get_pixel(20, 12));
    ASSERT_TRUE(screen.get_pixel(21, 12));
    ASSERT_TRUE(screen.get_pixel(22, 12));

    emulator->execute_next_instruction();
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->registers()[0xF], 0x01);

    ASSERT_FALSE(screen.get_pixel(15, 8));
    ASSERT_TRUE(screen.get_pixel(16, 8));
    ASSERT_TRUE(screen.get_pixel(17, 8));
    ASSERT_FALSE(screen.get_pixel(18, 8));
    ASSERT_FALSE(screen.get_pixel(19, 8));
    ASSERT_TRUE(screen.get_pixel(20, 8));
    ASSERT_TRUE(screen.get_pixel(21, 8));
    ASSERT_FALSE(screen.get_pixel(22, 8));
}

// EX9E: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is pressed
TEST_F(DefaultState, SkipIfKeyPressed) {
    input_source.pressed_keys.insert(emulator::Key::C);

    write_set_register_opcode(0xA, 0xB, 0x200); // set VA to 0xB
    write_opcode(0xEA9E, 0x202);                // skip if the key stored in VA (0xB) is being held down
    write_set_register_opcode(0x2, 42, 0x204);  // set V2 to 42
    write_set_register_opcode(0xB, 0xC, 0x206); // set VB to 0xC
    write_opcode(0xEB9E, 0x208);                // skip if the key stored in VB (0xC) is being held down
    write_set_register_opcode(0x3, 42, 0x20A);  // set V3 to 42

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
    ASSERT_EQ(emulator->registers()[0xA], 0xB);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x206);
    ASSERT_EQ(emulator->registers()[0x2], 42);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x208);
    ASSERT_EQ(emulator->registers()[0xB], 0xC);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x20C);
    ASSERT_EQ(emulator->registers()[0x3], 0); // unchanged
}

// EXA1: Skip the following instruction if the key corresponding to the hex value currently stored in register VX is not pressed
TEST_F(DefaultState, SkipIfKeyNotPressed) {
    input_source.pressed_keys.insert(emulator::Key::C);

    write_set_register_opcode(0xA, 0xB, 0x200); // set VA to 0xB
    write_opcode(0xEAA1, 0x202);                // skip if the key stored in VA (0xB) is not being held down
    write_set_register_opcode(0x2, 42, 0x204);  // set V2 to 42
    write_set_register_opcode(0xB, 0xC, 0x206); // set VB to 0xC
    write_opcode(0xEBA1, 0x208);                // skip if the key stored in VB (0xC) is not being held down
    write_set_register_opcode(0x3, 42, 0x20A);  // set V3 to 42

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
    ASSERT_EQ(emulator->registers()[0xA], 0xB);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x206);
    ASSERT_EQ(emulator->registers()[0x2], 0); // unchanged
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x208);
    ASSERT_EQ(emulator->registers()[0xB], 0xC);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x20A);
}

// FX0A: Wait for a keypress and store the result in register VX
TEST_F(DefaultState, AwaitKeypress) {
    input_source.to_be_pressed.push_back(emulator::Key::E);
    write_opcode(0xFB0A, 0x200); // await key and store in VB

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
    ASSERT_EQ(emulator->registers()[0xB], 0xE);
}

// FX15: Set the delay timer to the value of register VX
// FX07: Store the current value of the delay timer in register VX
TEST_F(DefaultState, SetAndReadDelayTimer) {
    write_set_register_opcode(0xA, 120, 0x200); // set VA to 120
    write_opcode(0xFA15, 0x202);                // set timer to the value of VA
    write_opcode(0xF807, 0x204);                // read timer value into V8
    write_opcode(0x1204, 0x206);                // jump to 0x204

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);
    ASSERT_EQ(emulator->registers()[0x8], 0); // unchanged
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x206);
    ASSERT_EQ(emulator->registers()[0x8], 120);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);

    for (int i = 1; i <= 120; ++i) {
        time_source.elapsed = static_cast<double>(i) * 1.0 / 60.0;
        ASSERT_EQ(emulator->instruction_pointer(), 0x204);
        emulator->execute_next_instruction();
        ASSERT_EQ(emulator->instruction_pointer(), 0x206);
        ASSERT_EQ(emulator->registers()[0x8], static_cast<u8>(120 - i));
        emulator->execute_next_instruction();
    }

    time_source.elapsed = 1000000;
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x206);
    ASSERT_EQ(emulator->registers()[0x8], 0);
}

// FX18: Set the sound timer to the value of register VX
TEST_F(DefaultState, SetAndReadSoundTimer) {
    write_set_register_opcode(0xA, 120, 0x200); // set VA to 120
    write_opcode(0xFA18, 0x202);                // set timer to the value of VA

    ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x202);
    ASSERT_EQ(emulator->registers()[0xA], 120);
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->instruction_pointer(), 0x204);
    ASSERT_EQ(emulator->sound_timer(), 120);

    time_source.elapsed = 2.0 / 120.0 * 119.0;
    ASSERT_EQ(emulator->sound_timer(), 1);
    time_source.elapsed = 2.0;
    ASSERT_EQ(emulator->sound_timer(), 0);
    time_source.elapsed = 100.0;
    ASSERT_EQ(emulator->sound_timer(), 0);
}

// FX1E: Add the value stored in register VX to register I
TEST_F(DefaultState, AddRegisterToAddressRegister) {
    write_set_register_opcode(0xA, 20, 0x200); // set VA to 20
    write_opcode(0xA016, 0x202);               // store 22 into address register
    write_opcode(0xFA1E, 0x204);               // add value of VA (20) to address register

    emulator->execute_next_instruction();
    emulator->execute_next_instruction();
    emulator->execute_next_instruction();

    ASSERT_EQ(emulator->address_register(), 42);
}

// FX29: Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
TEST_F(DefaultState, SetAddressRegisterToFontGlyph) {
    for (u8 register_ = 0x0; register_ <= 0xF; ++register_) {
        Address instruction_address = 0x200;
        auto const opcode = 0xF029 | (register_ << 8);
        for (u8 glyph = 0x0; glyph <= 0xF; ++glyph) {
            write_set_register_opcode(register_, glyph, instruction_address);
            instruction_address += 2;
            write_opcode(opcode, instruction_address);
            instruction_address += 2;
        }
        write_opcode(0x1200, instruction_address); // jump back to 0x200

        for (u8 glyph = 0x0; glyph <= 0xF; ++glyph) {
            emulator->execute_next_instruction();
            ASSERT_EQ(emulator->registers().at(register_), glyph);
            emulator->execute_next_instruction();
            ASSERT_EQ(emulator->address_register(), 5 * static_cast<Address>(glyph));
            for (usize i = 0; i < 5; ++i) {
                ASSERT_EQ(
                        emulator->read(gsl::narrow<Address>(emulator->address_register() + i)),
                        font_glyphs.at(glyph * 5 + i)
                );
            }
        }
        emulator->execute_next_instruction();
        ASSERT_EQ(emulator->instruction_pointer(), 0x200);
    }
}

// FX33: Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I+1, and I+2
TEST_F(DefaultState, BinaryCodedDecimal) {
    write_opcode(0xA050, 0x200); // set address register to 0x50 = 5 * 16 (behind the font glyphs)
    emulator->execute_next_instruction();
    ASSERT_EQ(emulator->address_register(), 5 * 16);
    for (u8 register_ = 0x0; register_ <= 0xF; ++register_) {
        Address instruction_address = 0x202;
        for (int value = 0; value <= 255; ++value) {
            write_set_register_opcode(register_, gsl::narrow<u8>(value), instruction_address);
            instruction_address += 2;
            auto const opcode = 0xF033 | (register_ << 8);
            write_opcode(opcode, instruction_address);
            instruction_address += 2;
        }
        write_opcode(0x1202, instruction_address); // jump back to 0x202

        for (int value = 0; value <= 255; ++value) {
            emulator->execute_next_instruction();
            ASSERT_EQ(emulator->registers().at(register_), value);
            emulator->execute_next_instruction();
            ASSERT_EQ(emulator->read(emulator->address_register() + 2), value % 10);
            ASSERT_EQ(emulator->read(emulator->address_register() + 1), (value / 10) % 10);
            ASSERT_EQ(emulator->read(emulator->address_register() + 0), value / 100);
        }
        emulator->execute_next_instruction(); // for the jump
    }
}

// FX55: Store the values of registers V0 to VX inclusive in memory starting at address I
//       I is set to I + X + 1 after operation
TEST_F(DefaultState, Store) {
    auto register_values = std::array<u8, 16>{};
    for (auto& value : register_values) {
        value = random_byte();
    }

    auto instruction_address = Address{ 0x200 };
    for (u8 register_ = 0x0; register_ <= 0xF; ++register_) {
        write_set_register_opcode(register_, register_values.at(register_), instruction_address);
        instruction_address += 2;
    }

    for (u8 bound = 0x0; bound <= 0xF; ++bound) {
        write_opcode(0xA050, instruction_address); // set address register to 0x50
        instruction_address += 2;
        auto const opcode = 0xF055 | (bound << 8);
        write_opcode(opcode, instruction_address);
        instruction_address += 2;
    }

    for (u8 register_ = 0x0; register_ <= 0xF; ++register_) {
        emulator->execute_next_instruction();
    }
    for (usize i = 0; i < register_values.size(); ++i) {
        ASSERT_EQ(register_values.at(i), emulator->registers().at(gsl::narrow<u8>(i)));
    }

    for (u8 bound = 0x0; bound <= 0xF; ++bound) {
        emulator->execute_next_instruction();
        ASSERT_EQ(emulator->address_register(), 0x50);

        emulator->execute_next_instruction();
        for (usize i = 0; i < register_values.size(); ++i) {
            auto const expected = (gsl::narrow<u8>(i) <= bound ? register_values.at(i) : 0);
            ASSERT_EQ(emulator->read(0x50 + i), expected);
        }
        ASSERT_EQ(emulator->address_register(), 0x50 + bound + 1);
    }
}

// FX65: Fill registers V0 to VX inclusive with the values stored in memory starting at address I
//       I is set to I + X + 1 after operation
TEST_F(DefaultState, Load) {
    for (Address address = 0; address < 16; ++address) {
        emulator->write(address + 0x50, random_byte());
    }

    auto instruction_address = Address{ 0x200 };
    for (u8 bound = 0x0; bound <= 0xF; ++bound) {
        write_opcode(0xA050, instruction_address); // set address register to 0x50
        instruction_address += 2;

        auto const opcode = 0xF065 | (bound << 8);
        write_opcode(opcode, instruction_address);
        instruction_address += 2;
    }

    for (u8 bound = 0x0; bound <= 0xF; ++bound) {
        emulator->execute_next_instruction();
        ASSERT_EQ(emulator->address_register(), 0x50);
        emulator->execute_next_instruction();
        for (u8 register_ = 0; register_ <= 0xF; ++register_) {
            auto const expected = (register_ <= bound ? emulator->read(0x50 + register_) : 0);
            ASSERT_EQ(emulator->registers().at(register_), expected);
        }
        ASSERT_EQ(emulator->address_register(), 0x50 + bound + 1);
    }
}
