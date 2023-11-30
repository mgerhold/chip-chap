#include "application.hpp"
#include "renderer.hpp"

#include <cstdlib>

class ChipChap final : public Application {
protected:
    void update([[maybe_unused]] double delta_seconds) override { }

    void render(Renderer& renderer) const override {
        renderer.clear(Color{ 0, 144, 158, 255 }).flush();
    }
};

int main(int, char**) {
    auto chip_chap = ChipChap{};
    chip_chap.run();
    return EXIT_SUCCESS;
}
