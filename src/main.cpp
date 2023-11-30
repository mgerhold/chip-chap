#include "renderer.hpp"
#include "visitor.hpp"
#include "window.hpp"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <cstdlib>
#include <imgui.h>

int main(int, char**) {
    auto window = Window{ 800, 600, "ChipChap" };

    auto running = true;
    while (running) {
        window.update();
        while (auto event = window.next_event()) {
            visit(
                    event.value(),
                    [&](event::Quit const&) { running = false; },
                    [&](event::KeyDown const& key) {
                        if (key.which == KeyCode::Escape) {
                            running = false;
                        }
                    },
                    [&](event::KeyUp const&) {}
            );
        }

        window.renderer().clear(Color{ 0xCC, 0xAB, 0xD8, 0xFF }).swap();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window.sdl_window());
    }

    return EXIT_SUCCESS;
}
