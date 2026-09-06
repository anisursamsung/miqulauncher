#include "config_manager.hpp"
#include <miqutoolkit/miqutoolkit.hpp>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <algorithm>

namespace miqu {

namespace fs = std::filesystem;

static std::string trim_str(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\"'");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n\"'");
    return str.substr(first, (last - first + 1));
}

std::string ConfigManager::get_user_config_path() {
    const char* xdg_config = getenv("XDG_CONFIG_HOME");
    const char* home = getenv("HOME");
    if (xdg_config && *xdg_config) {
        return std::string(xdg_config) + "/miqulauncher/miqulauncher.conf";
    }
    if (home && *home) {
        return std::string(home) + "/.config/miqulauncher/miqulauncher.conf";
    }
    return "";
}

void ConfigManager::load(LauncherConfig& config) {
    std::string user_path = get_user_config_path();
    std::string sys_path = "/usr/share/miqulauncher/miqulauncher.conf";

    std::string target_path;
    if (!config.config_path.empty() && fs::exists(config.config_path)) {
        target_path = config.config_path;
    } else if (!user_path.empty() && fs::exists(user_path)) {
        target_path = user_path;
    } else if (fs::exists(sys_path)) {
        target_path = sys_path;
    }

    if (target_path.empty()) {
        return;
    }

    // 1. Let Config load all colors and metrics from this config file
    Config::get()->load_from_file(target_path);

    // 2. Parse launcher-specific layout properties
    std::ifstream file(target_path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        line = trim_str(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = trim_str(line.substr(0, eq_pos));
        std::string val = trim_str(line.substr(eq_pos + 1));

        size_t c_pos = val.find('#');
        if (c_pos != std::string::npos) {
            val = trim_str(val.substr(0, c_pos));
        }

        if (key == "width") {
            try { config.width = std::max(200, std::stoi(val)); } catch (...) {}
        } else if (key == "height") {
            try { config.height = std::max(150, std::stoi(val)); } catch (...) {}
        } else if (key == "cell_size") {
            try { config.cell_size = std::max(30, std::stoi(val)); } catch (...) {}
        } else if (key == "spacing") {
            try { config.spacing = std::max(0, std::stoi(val)); } catch (...) {}
        }
    }
}

} // namespace miqu
