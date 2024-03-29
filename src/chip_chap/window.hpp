#pragma once

#include "event.hpp"
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <common/types.hpp>
#include <vector>

struct SDL_Window;

class WindowError final : public std::runtime_error {
public:
    explicit WindowError(std::string_view message);
};

class Window final {
    friend class Application;

private:
    SDL_Window* m_window;
    SDL_GLContext m_gl_context;
    std::deque<event::Event> m_events;

public:
    Window(int width, int height, std::string const& title);
    Window(Window const& other) = delete;
    Window(Window&& other) noexcept = delete;
    Window& operator=(Window const& other) = delete;
    Window& operator=(Window&& other) noexcept = delete;
    void update();
    [[nodiscard]] Optional<event::Event> next_event();
    ~Window();

private:
    [[nodiscard]] SDL_Window* sdl_window() const {
        return m_window;
    }
};
