#include <miqutoolkit/miqutoolkit.hpp>
#include "ui/launcher_window.hpp"
#include "system/cli_parser.hpp"
#include "system/config_manager.hpp"

int main(int argc, char* argv[]) {
    miqu::LauncherConfig config;

    // Scan if custom config file path was specified on CLI
    for (int i = 1; i + 1 < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-c" || arg == "--config" || arg == "-config") {
            config.config_path = argv[i + 1];
            break;
        }
    }

    // Load independent launcher configuration & theme
    miqu::ConfigManager::load(config);

    // Parse CLI options (CLI options override config file settings)
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
