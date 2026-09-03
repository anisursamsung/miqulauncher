#include "run_provider.hpp"
#include "system/binary_manager.hpp"
#include <miqutoolkit/view/image_view.hpp>
#include <algorithm>

namespace miqu {

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

RunProvider::RunProvider() {
    m_icon_path = ImageView::resolve_icon_path("system-run");
    if (m_icon_path.empty()) {
        m_icon_path = ImageView::resolve_icon_path("utilities-terminal");
    }
}

std::vector<LauncherItem> RunProvider::get_items(const std::string& query) const {
    const auto& binaries = BinaryManager::get_system_binaries();
    std::vector<LauncherItem> items;
    const size_t MAX_RESULTS = 80;
    items.reserve(MAX_RESULTS);

    std::string lower_query = to_lower(query);

    if (lower_query.empty()) {
        for (size_t i = 0; i < binaries.size() && items.size() < MAX_RESULTS; ++i) {
            LauncherItem info;
            info.id = binaries[i];
            info.title = binaries[i];
            info.terminal = BinaryManager::is_terminal_command(binaries[i]);
            info.subtitle = info.terminal ? "Terminal Command" : "System Binary";
            info.icon_name = info.terminal ? "utilities-terminal" : "system-run";
            info.icon_path = m_icon_path;
            info.exec_cmd = binaries[i];
            items.push_back(std::move(info));
        }
    } else {
        std::vector<const std::string*> prefix_matches;
        std::vector<const std::string*> contains_matches;

        for (const auto& bin : binaries) {
            std::string lower_bin = to_lower(bin);
            if (lower_bin.rfind(lower_query, 0) == 0) {
                prefix_matches.push_back(&bin);
                if (prefix_matches.size() >= MAX_RESULTS) break;
            } else if (lower_bin.find(lower_query) != std::string::npos) {
                contains_matches.push_back(&bin);
            }
        }

        for (const auto* pbin : prefix_matches) {
            if (items.size() >= MAX_RESULTS) break;
            LauncherItem info;
            info.id = *pbin;
            info.title = *pbin;
            info.terminal = BinaryManager::is_terminal_command(*pbin);
            info.subtitle = info.terminal ? "Terminal Command" : "System Binary";
            info.icon_name = info.terminal ? "utilities-terminal" : "system-run";
            info.icon_path = m_icon_path;
            info.exec_cmd = *pbin;
            items.push_back(std::move(info));
        }

        for (const auto* pbin : contains_matches) {
            if (items.size() >= MAX_RESULTS) break;
            LauncherItem info;
            info.id = *pbin;
            info.title = *pbin;
            info.terminal = BinaryManager::is_terminal_command(*pbin);
            info.subtitle = info.terminal ? "Terminal Command" : "System Binary";
            info.icon_name = info.terminal ? "utilities-terminal" : "system-run";
            info.icon_path = m_icon_path;
            info.exec_cmd = *pbin;
            items.push_back(std::move(info));
        }
    }

    return items;
}

void RunProvider::launch(const LauncherItem& item) const {
    BinaryManager::launch_command(item.exec_cmd, item.terminal);
}

void RunProvider::launch_raw(const std::string& cmd) const {
    BinaryManager::launch_command(cmd, false);
}

} // namespace miqu
