#include "renderer.hpp"
#include "color.hpp"
#include <SDL.h>

Renderer::Renderer(SDL_Renderer* renderer) : m_renderer{ renderer } { }

void Renderer::clear(Color const color) const {
    set_color(color);
    SDL_RenderClear(m_renderer);
}

void Renderer::swap() const {
    SDL_RenderPresent(m_renderer);
}

void Renderer::set_color(Color color) const {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
}
