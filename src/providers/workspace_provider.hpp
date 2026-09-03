#pragma once

#include "model/launcher_item.hpp"
#include <miqutoolkit/system/workspace_manager.hpp>
#include <vector>
#include <string>
#include <functional>

namespace miqu {

class WorkspaceProvider {
public:
    WorkspaceProvider();

    std::vector<LauncherItem> get_items(const std::string& query = "", int* out_active_index = nullptr) const;
    void activate(const LauncherItem& item) const;
    void on_workspaces_changed(std::function<void()> cb);

private:
    std::string m_icon_path;
};

} // namespace miqu
