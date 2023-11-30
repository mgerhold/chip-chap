#pragma once

#include "window.hpp"

class Application {
private:
    Window m_window;
    bool m_running = true;

public:
    Application();
    Application(Application const& other) = delete;
    Application(Application&& other) noexcept = delete;
    Application& operator=(Application const& other) = delete;
    Application& operator=(Application&& other) noexcept = delete;

    virtual ~Application() = default;

    void run();
protected:
    virtual void update(double delta_seconds) = 0;
    virtual void render(Renderer& renderer) const = 0;
    void quit();
};
