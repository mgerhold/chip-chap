#include "screen.hpp"
#include <cstring>

static constexpr auto visible = 0xFFFFFFFF;
static constexpr auto invisible = 0x000000FF;

bool Screen::set_pixel(u8 const x, u8 const y, bool is_set) {
    auto const previous_value = get_pixel(x, y);
    m_pixels.at(coordinates_to_offset(x, y)) = (is_set ? visible : invisible);
    return previous_value;
}

[[nodiscard]] bool Screen::get_pixel(u8 const x, u8 const y) const {
    return m_pixels.at(coordinates_to_offset(x, y)) == visible;
}

[[nodiscard]] usize Screen::width() const {
    return screen_width;
}

[[nodiscard]] usize Screen::height() const {
    return screen_height;
}

void Screen::clear() {
    std::memset(m_pixels.data(), 0, buffer_size_in_bytes);
}

[[nodiscard]] std::span<std::byte const> Screen::raw_data() const {
    auto const begin = reinterpret_cast<std::byte const*>(m_pixels.data());
    return std::span{ begin, buffer_size_in_bytes };
}

[[nodiscard]] usize Screen::coordinates_to_offset(u8 const x, u8 const y) const {
    return x + width() * y;
}
