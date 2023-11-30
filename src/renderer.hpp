#pragma once


struct Color;
struct SDL_Renderer;

class Renderer final {
    friend class Window;

private:
    SDL_Renderer* m_renderer;

    explicit Renderer(SDL_Renderer* renderer);

public:
    Renderer(Renderer const& other) = delete;
    Renderer(Renderer&& other) noexcept = delete;
    Renderer& operator=(Renderer const& other) = delete;
    Renderer& operator=(Renderer&& other) noexcept = delete;

    void clear(Color color) const;
    void swap() const;

private:
    void set_color(Color color) const;
};
