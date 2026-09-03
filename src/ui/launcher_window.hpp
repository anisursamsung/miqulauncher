#pragma once

#include <miqutoolkit/miqutoolkit.hpp>
#include "providers/app_provider.hpp"
#include "providers/window_provider.hpp"
#include "providers/workspace_provider.hpp"
#include "providers/run_provider.hpp"
#include <array>
#include <memory>

namespace miqu {

class LauncherWindow {
public:
    explicit LauncherWindow(AppEngine* engine);
    ~LauncherWindow() = default;

    bool init();

private:
    void switch_tab(int tab_index);
    void refresh_current_tab();
    void handle_item_click(size_t index, std::shared_ptr<View> view);
    void handle_submit(const std::string& query);

    AppEngine* m_engine = nullptr;
    std::shared_ptr<Window> m_window;
    std::shared_ptr<GridView> m_grid;
    std::shared_ptr<SearchView> m_search;
    std::array<std::shared_ptr<Button>, 4> m_tab_buttons;

    AppProvider m_app_provider;
    WindowProvider m_window_provider;
    WorkspaceProvider m_workspace_provider;
    RunProvider m_run_provider;

    int m_active_tab = 0;
};

} // namespace miqu
