#include "script_provider.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>

namespace miqu {

static std::string expand_path(const std::string& path) {
    if (!path.empty() && path[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            return std::string(home) + path.substr(1);
        }
    }
    return path;
}

ScriptProvider::ScriptProvider(std::string script_path)
    : m_script_path(std::move(script_path)) {
    refresh();
}

void ScriptProvider::refresh() {
    m_raw_items.clear();
    std::string full_path = expand_path(m_script_path);

    FILE* pipe = popen(full_path.c_str(), "r");
    if (!pipe) return;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
            line.pop_back();
        }
        if (!line.empty()) {
            m_raw_items.push_back(line);
        }
    }
    pclose(pipe);
}

std::vector<LauncherItem> ScriptProvider::query(const std::string& filter_text) {
    std::string lower_filter = filter_text;
    std::transform(lower_filter.begin(), lower_filter.end(), lower_filter.begin(), ::tolower);

    std::vector<LauncherItem> results;
    for (size_t i = 0; i < m_raw_items.size(); ++i) {
        const auto& line = m_raw_items[i];
        // Check for rofi icon format: text\0icon\x1ficonname
        std::string title = line;
        std::string icon = "application-x-executable";

        size_t null_pos = line.find('\0');
        if (null_pos != std::string::npos) {
            title = line.substr(0, null_pos);
            size_t icon_pos = line.find("\x1f", null_pos);
            if (icon_pos != std::string::npos) {
                icon = line.substr(icon_pos + 1);
            }
        }

        if (!filter_text.empty()) {
            std::string lower_title = title;
            std::transform(lower_title.begin(), lower_title.end(), lower_title.begin(), ::tolower);
            if (lower_title.find(lower_filter) == std::string::npos) {
                continue;
            }
        }

        LauncherItem item;
        item.id = std::to_string(i);
        item.title = title;
        item.subtitle = "";
        item.icon_name = icon;
        results.push_back(item);
    }
    return results;
}

void ScriptProvider::activate(const LauncherItem& item) {
    std::string full_path = expand_path(m_script_path);
    std::string cmd = full_path + " \"" + item.title + "\" &";

    pid_t pid = fork();
    if (pid == 0) {
        setsid();
        execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
        _exit(127);
    }
}

} // namespace miqu
