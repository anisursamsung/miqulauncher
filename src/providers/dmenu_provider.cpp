#include "dmenu_provider.hpp"
#include <iostream>
#include <algorithm>
#include <unistd.h>

namespace miqu {

DmenuProvider::DmenuProvider() {}

void DmenuProvider::load_from_stdin() {
    if (isatty(STDIN_FILENO)) {
        return;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            m_raw_items.push_back(line);
        }
    }
}

std::vector<LauncherItem> DmenuProvider::query(const std::string& filter_text) {
    std::string lower_filter = filter_text;
    std::transform(lower_filter.begin(), lower_filter.end(), lower_filter.begin(), ::tolower);

    std::vector<LauncherItem> results;
    for (size_t i = 0; i < m_raw_items.size(); ++i) {
        const auto& text = m_raw_items[i];
        if (!filter_text.empty()) {
            std::string lower_text = text;
            std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
            if (lower_text.find(lower_filter) == std::string::npos) {
                continue;
            }
        }

        LauncherItem item;
        item.id = std::to_string(i);
        item.title = text;
        item.subtitle = "";
        item.icon_name = "application-x-executable";
        results.push_back(item);
    }
    return results;
}

void DmenuProvider::activate(const LauncherItem& item) {
    std::cout << item.title << std::endl;
}

void DmenuProvider::submit(const std::string& raw_text) {
    std::cout << raw_text << std::endl;
}

} // namespace miqu
