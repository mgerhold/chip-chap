#include "application.hpp"
#include "renderer.hpp"
#include "visitor.hpp"
#include <SDL.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <chrono>

Application::Application() : m_window{ 800, 600, "MyWindow" } { }

void Application::run() {
    using Clock = std::chrono::steady_clock;
    auto last = Clock::now();
    auto renderer = m_window.renderer();
    while (m_running) {
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
        auto const delta_seconds =
                static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()) / 1000000.0;
        update(delta_seconds);
        render(renderer);

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

void Application::quit() {
    m_running = false;
}
