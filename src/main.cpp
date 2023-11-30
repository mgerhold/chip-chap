#include "window.hpp"

#include "overloaded.hpp"
#include <SDL_keycode.h>
#include <cstdlib>

int main(int, char**) {
    auto window = Window{ 800, 600, "ChipChap" };

    auto running = true;
    while (running) {
        window.update();
        while (auto event = window.next_event()) {
            std::visit(
                    Overloaded{
                            [&](event::Quit const&) { running = false; },
                            [&](event::KeyDown const& key) {
                                if (key.which == KeyCode::Escape) {
                                    running = false;
                                }
                            },
                            [&](event::KeyUp const&) {},
                    },
                    event.value()
            );
        }
    }

    return EXIT_SUCCESS;
}
