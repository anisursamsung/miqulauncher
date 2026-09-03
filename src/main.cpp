#include <miqutoolkit/miqutoolkit.hpp>
#include "ui/launcher_window.hpp"
#include "system/cli_parser.hpp"

int main(int argc, char* argv[]) {
    miqu::LauncherConfig config;
    auto parse_res = miqu::CliParser::parse(argc, argv, config);
    if (parse_res == miqu::CliParser::ParseResult::ExitSuccess) {
        return 0;
    }
    if (parse_res == miqu::CliParser::ParseResult::Error) {
        return 1;
    }

    auto engine = miqu::AppEngine::create();
    if (!engine) {
        return 1;
    }

    auto launcher = std::make_shared<miqu::LauncherWindow>(engine.get(), std::move(config));
    if (!launcher->init()) {
        return 1;
    }

    return engine->enter_loop();
}
