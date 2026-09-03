#include "app_provider.hpp"
#include "system/package_manager.hpp"
#include <thread>
#include <algorithm>

namespace miqu {

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

void AppProvider::load_async(std::function<void()> on_loaded) {
    std::thread([this, on_loaded]() {
        m_all_apps = PackageManager::get_installed_applications();
        m_loaded = true;
        if (on_loaded) {
            on_loaded();
        }
    }).detach();
}

std::vector<LauncherItem> AppProvider::get_items(const std::string& query) const {
    if (query.empty()) {
        return m_all_apps;
    }

    std::string lower_query = to_lower(query);
    std::vector<LauncherItem> filtered;
    for (const auto& app : m_all_apps) {
        if (to_lower(app.title).find(lower_query) != std::string::npos ||
            to_lower(app.subtitle).find(lower_query) != std::string::npos ||
            to_lower(app.exec_cmd).find(lower_query) != std::string::npos ||
            to_lower(app.id).find(lower_query) != std::string::npos) {
            filtered.push_back(app);
        }
    }
    return filtered;
}

void AppProvider::launch(const LauncherItem& item) const {
    PackageManager::launch(item);
}

} // namespace miqu
