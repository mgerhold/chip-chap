#include "window.hpp"
#include "event.hpp"
#include <SDL.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <format>
#include <glad/glad.h>
#include <imgui.h>

WindowError::WindowError(std::string_view const message)
    : std::runtime_error{ std::format("failed to create window: {}", message) } { }

Window::Window(int const width, int const height, std::string const& title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        throw WindowError{ std::format("could not initialize SDL ('{}')", SDL_GetError()) };
    }
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // Set major version of OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0); // Set minor version of OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    m_window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (m_window == nullptr) {
        SDL_Quit();
        throw WindowError{ SDL_GetError() };
    }

    m_gl_context = SDL_GL_CreateContext(m_window);
    if (m_gl_context == nullptr) {
        SDL_Quit();
        SDL_DestroyWindow(m_window);
        throw WindowError{ std::format("failed to create OpenGL context ('{}')", SDL_GetError()) };
    }

    SDL_GL_SetSwapInterval(0); // no V-Sync

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        SDL_GL_DeleteContext(m_gl_context);
        SDL_Quit();
        SDL_DestroyWindow(m_window);
        throw WindowError{ "failed to initialize GLAD" };
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void Window::update() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                m_events.emplace_back(event::Quit{});
                break;
            case SDL_KEYDOWN:
                if (event.key.repeat == 0) {
                    m_events.emplace_back(event::KeyDown{ static_cast<KeyCode>(event.key.keysym.sym) });
                }
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(m_gl_context);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
