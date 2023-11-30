#include "window.hpp"
#include "SDL.h"
#include "event.hpp"
#include <format>

WindowError::WindowError(std::string_view const message)
    : std::runtime_error{ std::format("failed to create window: {}", message) } { }

Window::Window(int const width, int const height, std::string const& title) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw WindowError{ std::format("could not initialize SDL ('{}')", SDL_GetError()) };
    }
    m_window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_SHOWN
    );
    if (m_window == nullptr) {
        SDL_Quit();
        throw WindowError{ SDL_GetError() };
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (m_renderer == nullptr) {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        throw WindowError{ std::format("could not create renderer ('{}')", SDL_GetError()) };
    }
}

void Window::update() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_events.emplace_back(event::Quit{});
                break;
            case SDL_KEYDOWN:
                m_events.emplace_back(event::KeyDown{ static_cast<KeyCode>(event.key.keysym.sym) });
                break;
            case SDL_KEYUP:
                m_events.emplace_back(event::KeyUp{ static_cast<KeyCode>(event.key.keysym.sym) });
                break;
        }
    }
}

[[nodiscard]] Optional<event::Event> Window::next_event() {
    if (m_events.empty()) {
        return none;
    }
    auto result = m_events.front();
    m_events.pop_front();
    return result;
}

Window::~Window() {
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
