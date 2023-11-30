#include "application.hpp"
#include "renderer.hpp"
#include "visitor.hpp"
#include <SDL.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>

Application::Application() : m_window{ 800, 600, "MyWindow" } { }

void Application::run() {
    auto last = SDL_GetTicks();
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
        auto const current = SDL_GetTicks();
        if (current == last) {
            continue;
        }
        auto const delta_seconds = static_cast<double>(current - last) / 1000.0;
        update(delta_seconds);
        render(renderer);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(m_window.sdl_window());

        last = current;
    }
}

void Application::quit() {
    m_running = false;
}
