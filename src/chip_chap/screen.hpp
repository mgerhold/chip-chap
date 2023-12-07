#pragma once

#include <array>
#include <chip8/basic_screen.hpp>
#include <span>

class Screen final : public emulator::BasicScreen {
private:
    static constexpr auto screen_width = 64;
    static constexpr auto screen_height = 32;

    std::array<u32, screen_width * screen_height> m_pixels{};
    static constexpr auto buffer_size_in_bytes =
            sizeof(decltype(m_pixels)::value_type) * std::tuple_size<decltype(m_pixels)>{};

public:
    bool set_pixel(u8 x, u8 y, bool is_set) override;
    [[nodiscard]] bool get_pixel(u8 x, u8 y) const override;
    [[nodiscard]] usize width() const override;
    [[nodiscard]] usize height() const override;
    void clear() override;
    [[nodiscard]] std::span<std::byte const> raw_data() const;

private:
    [[nodiscard]] usize coordinates_to_offset(u8 x, u8 y) const;
};
