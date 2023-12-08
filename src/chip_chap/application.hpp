#pragma once

#include "window.hpp"

class Application {
private:
    Window m_window;
    bool m_running = true;
    double m_elapsed_seconds = 0.0;
    double m_delta_seconds = 0.0;

public:
    Application();
    Application(Application const& other) = delete;
    Application(Application&& other) noexcept = delete;
    Application& operator=(Application const& other) = delete;
    Application& operator=(Application&& other) noexcept = delete;

    virtual ~Application() = default;

    void run();
protected:
    virtual void update() = 0;
    virtual void imgui_render() = 0;
    virtual void handle_event(event::Event const& event) = 0;
    [[nodiscard]] double elapsed_seconds() const;
    [[nodiscard]] double delta_seconds() const;
    void quit();
};
