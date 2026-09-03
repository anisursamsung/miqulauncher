#include <miqutoolkit/miqutoolkit.hpp>
#include "ui/launcher_window.hpp"

int main(int argc, char* argv[]) {
    auto engine = miqu::AppEngine::create();
    if (!engine) {
        return 1;
    }

    auto launcher = std::make_shared<miqu::LauncherWindow>(engine.get());
    if (!launcher->init()) {
        return 1;
    }

    return engine->enter_loop();
}
