#include "application.hpp"
#include "visitor.hpp"
#include <SDL.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <chrono>

Application::Application() : m_window{ 800, 600, "MyWindow" } { }

void Application::run() {
    using Clock = std::chrono::steady_clock;
    auto const start = Clock::now();
    auto last = start;

    while (m_running) {
        using std::chrono::duration_cast, std::chrono::microseconds;

        m_window.update();
        while (auto event = m_window.next_event()) {
            visit(
                    event.value(),
                    [&](event::Quit const&) { m_running = false; },
                    [&](event::KeyDown const&) {},
                    [&](event::KeyUp const&) {}
            );
        }
        auto const current = Clock::now();
        auto const elapsed = current - last;
        m_delta_seconds = static_cast<double>(duration_cast<microseconds>(elapsed).count()) / 1000000.0;
        m_elapsed_seconds = static_cast<double>(duration_cast<microseconds>(current - start).count()) / 1000000.0;
        update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        imgui_render();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(m_window.sdl_window());

        last = current;
    }
}

[[nodiscard]] double Application::elapsed_seconds() const {
    return m_elapsed_seconds;
}

[[nodiscard]] double Application::delta_seconds() const {
    return m_delta_seconds;
}

void Application::quit() {
    m_running = false;
}
