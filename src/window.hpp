#pragma once

#include "event.hpp"
#include "types.hpp"
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;
class Renderer;

class WindowError final : public std::runtime_error {
public:
    explicit WindowError(std::string_view message);
};

class Window final {
private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
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
    [[nodiscard]] Renderer renderer() const;
    ~Window();

    // todo: make private + friend
    [[nodiscard]] SDL_Window* sdl_window() const {
        return m_window;
    }
};
