#include "workspace_provider.hpp"
#include <miqutoolkit/view/image_view.hpp>
#include <algorithm>

namespace miqu {

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

WorkspaceProvider::WorkspaceProvider() {
    m_icon_path = ImageView::resolve_icon_path("preferences-desktop-workspaces");
    if (m_icon_path.empty()) {
        m_icon_path = ImageView::resolve_icon_path("desktop");
    }
}

std::vector<LauncherItem> WorkspaceProvider::get_items(const std::string& query, int* out_active_index) const {
    auto workspaces = WorkspaceManager::get()->get_workspaces();
    std::vector<LauncherItem> items;
    items.reserve(workspaces.size());

    int active_idx = -1;
    for (size_t i = 0; i < workspaces.size(); ++i) {
        if (workspaces[i].is_active) {
            active_idx = static_cast<int>(i);
        }
        LauncherItem item;
        item.id = std::to_string(workspaces[i].id);
        item.title = "Workspace " + (!workspaces[i].name.empty() ? workspaces[i].name : std::to_string(workspaces[i].id));
        if (workspaces[i].is_active) {
            item.subtitle = "● Active";
        } else if (!workspaces[i].is_empty) {
            item.subtitle = "Occupied";
        } else {
            item.subtitle = "Empty";
        }
        item.icon_name = "preferences-desktop-workspaces";
        item.icon_path = m_icon_path;
        items.push_back(std::move(item));
    }

    if (out_active_index) {
        *out_active_index = active_idx;
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

void WorkspaceProvider::activate(const LauncherItem& item) const {
    try {
        size_t ws_id = std::stoull(item.id);
        WorkspaceManager::get()->activate_workspace(ws_id);
    } catch (...) {}
}

void WorkspaceProvider::on_workspaces_changed(std::function<void()> cb) {
    WorkspaceManager::get()->on_workspaces_changed(std::move(cb));
}

} // namespace miqu
