#include "input_source.hpp"
#include <algorithm>
#include <cassert>
#include <common/visitor.hpp>
#include <gsl/gsl>

InputSource::InputSource(std::array<KeyCode, 16> const& key_mapping) : m_key_mapping{ key_mapping } { }

void InputSource::await_keypress(std::function<void(emulator::Key)> callback) {
    m_blocking_input_callback = std::move(callback);
}

[[nodiscard]] bool InputSource::is_key_pressed(emulator::Key const key) {
    return m_pressed_keys.at(std::to_underlying(key));
}

void InputSource::handle_event(event::Event const& event) {
    visit(
            event,
            [&](event::KeyDown const& key_down_event) {
                auto const value = key_code_to_value(key_down_event.which);
                if (not value.has_value()) {
                    return;
                }
                assert(not m_pressed_keys.at(value.value()));
                m_pressed_keys.at(value.value()) = true;
                if (is_blocked()) {
                    m_blocking_input_callback(static_cast<emulator::Key>(value.value()));
                    m_blocking_input_callback = nullptr;
                }
            },
            [&](event::KeyUp const& key_up_event) {
                auto const value = key_code_to_value(key_up_event.which);
                if (not value.has_value()) {
                    return;
                }
                assert(m_pressed_keys.at(value.value()));
                m_pressed_keys.at(value.value()) = false;
            },
            [&](event::Quit const&) {}
    );
}

[[nodiscard]] bool InputSource::is_blocked() const {
    return m_blocking_input_callback != nullptr;
}

[[nodiscard]] bool InputSource::key_state(emulator::Key const key) const {
    return m_pressed_keys.at(std::to_underlying(key));
}

[[nodiscard]] Optional<u8> InputSource::key_code_to_value(KeyCode const key_code) const {
    auto const find_iterator = std::find(m_key_mapping.cbegin(), m_key_mapping.cend(), key_code);
    if (find_iterator == m_key_mapping.cend()) {
        return none;
    }
    return gsl::narrow<u8>(std::distance(m_key_mapping.cbegin(), find_iterator));
}
