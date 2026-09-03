#include "window_provider.hpp"
#include <miqutoolkit/view/image_view.hpp>
#include <algorithm>

namespace miqu {

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::vector<LauncherItem> WindowProvider::get_items(const std::string& query) const {
    auto windows = WindowManager::get()->get_windows();
    std::vector<LauncherItem> items;
    items.reserve(windows.size());

    for (const auto& win : windows) {
        LauncherItem item;
        item.id = std::to_string(win.id);
        item.title = !win.title.empty() ? win.title : (!win.app_id.empty() ? win.app_id : "Window");
        item.subtitle = win.app_id.empty() ? (win.is_active ? "Active" : "") : (win.app_id + (win.is_active ? " • Active" : ""));
        item.icon_name = win.app_id;
        item.icon_path = ImageView::resolve_icon_path(win.app_id);
        items.push_back(std::move(item));
    }

    if (query.empty()) {
        return items;
    }

    std::string lower_query = to_lower(query);
    std::vector<LauncherItem> filtered;
    for (const auto& item : items) {
        if (to_lower(item.title).find(lower_query) != std::string::npos ||
            to_lower(item.subtitle).find(lower_query) != std::string::npos ||
            to_lower(item.id).find(lower_query) != std::string::npos) {
            filtered.push_back(item);
        }
    }
    return filtered;
}

void WindowProvider::activate(const LauncherItem& item) const {
    auto windows = WindowManager::get()->get_windows();
    for (auto& win : windows) {
        if (std::to_string(win.id) == item.id) {
            win.activate();
            break;
        }
    }
}

void WindowProvider::on_windows_changed(std::function<void()> cb) {
    WindowManager::get()->on_windows_changed(std::move(cb));
}

} // namespace miqu
