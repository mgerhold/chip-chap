#include "renderer.hpp"
#include <SDL.h>

Renderer::Renderer(SDL_Renderer* renderer) : m_renderer{ renderer } { }

Renderer& Renderer::clear(Color const color) {
    set_color(color);
    SDL_RenderClear(m_renderer);
    return *this;
}

void Renderer::swap() {
    SDL_RenderFlush(m_renderer);
}

void Renderer::set_color(Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
}
