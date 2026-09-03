#pragma once

#include "model/launcher_item.hpp"
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace miqu {

class AppProvider {
public:
    AppProvider() = default;

    void load_async(std::function<void()> on_loaded = nullptr);
    std::vector<LauncherItem> get_items(const std::string& query = "") const;
    void launch(const LauncherItem& item) const;

private:
    std::vector<LauncherItem> m_all_apps;
    bool m_loaded = false;
};

} // namespace miqu
