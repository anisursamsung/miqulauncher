#include <miqutoolkit/miqutoolkit.hpp>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <thread>
#include <array>
#include <functional>

using namespace miqu;

int main(int argc, char* argv[]) {
    auto engine = AppEngine::create();
    if (!engine) {
        return 1;
    }

    auto theme = ColorScheme::get();

    // 1. Responsive Application Grid (autoFit columns)
    auto grid = GridViewBuilder::create()
        ->autoFit(100)
        ->cellHeight(100)
        ->spacing(10, 10)
        ->onItemClick([&](const GridItem& app) {
            PackageManager::launch(app);
            engine->quit();
        })
        ->build();

    // 2. Responsive Open Windows Grid
    auto windowsGrid = GridViewBuilder::create()
        ->autoFit(100)
        ->cellHeight(100)
        ->spacing(10, 10)
        ->onItemClick([&](const GridItem& item) {
            auto windows = WindowManager::get()->get_windows();
            for (auto& win : windows) {
                if (std::to_string(win.id) == item.id) {
                    win.activate();
                    break;
                }
            }
            engine->quit();
        })
        ->build();

    windowsGrid->set_visibility(Visibility::Gone);

    // 3. Workspaces Grid
    auto workspacesGrid = GridViewBuilder::create()
        ->autoFit(100)
        ->cellHeight(100)
        ->spacing(10, 10)
        ->onItemClick([&](const GridItem& item) {
            try {
                size_t ws_id = std::stoull(item.id);
                WorkspaceManager::get()->activate_workspace(ws_id);
            } catch (...) {}
            engine->quit();
        })
        ->build();

    workspacesGrid->set_visibility(Visibility::Gone);

    // 4. Run Command Grid
    auto runGrid = GridViewBuilder::create()
        ->autoFit(100)
        ->cellHeight(100)
        ->spacing(10, 10)
        ->onItemClick([&](const GridItem& item) {
            BinaryManager::launch_command(item.exec_cmd, item.terminal);
            engine->quit();
        })
        ->build();

    runGrid->set_visibility(Visibility::Gone);

    // 5. Tab Content Stack (holds active tab view)
    auto contentStack = FrameLayoutBuilder::create()
        ->addView(grid, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::MatchParent)))
        ->addView(windowsGrid, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::MatchParent)))
        ->addView(workspacesGrid, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::MatchParent)))
        ->addView(runGrid, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::MatchParent)))
        ->build();

    // 6. Tab Definitions and Switching Logic
    std::array<std::shared_ptr<Button>, 4> tabButtons;
    std::array<std::shared_ptr<View>, 4> tabViews = { grid, windowsGrid, workspacesGrid, runGrid };

    const char* tabTitles[4] = { "Apps", "Windows", "Workspaces", "Run" };
    const char* tabHints[4] = {
        "Search applications...",
        "Switch to open window...",
        "Switch or filter workspace...",
        "Run command line or binary..."
    };

    int activeTab = 0;
    std::shared_ptr<Window> window = nullptr;

    // Helper to refresh open windows list
    auto refreshWindows = [&]() {
        auto windows = WindowManager::get()->get_windows();
        std::vector<GridItem> items;
        items.reserve(windows.size());
        for (const auto& win : windows) {
            items.push_back(win.to_grid_item());
        }
        windowsGrid->set_adapter(std::move(items));
        if (window) {
            window->schedule_redraw();
        }
    };

    // Helper to refresh workspaces list
    auto refreshWorkspaces = [&]() {
        auto workspaces = WorkspaceManager::get()->get_workspaces();
        std::vector<GridItem> items;
        items.reserve(workspaces.size());
        int active_idx = -1;
        for (size_t i = 0; i < workspaces.size(); ++i) {
            if (workspaces[i].is_active) {
                active_idx = static_cast<int>(i);
            }
            items.push_back(workspaces[i].to_grid_item());
        }
        workspacesGrid->set_adapter(std::move(items));
        if (active_idx >= 0) {
            workspacesGrid->set_selected_index(active_idx);
        }
        if (window) {
            window->schedule_redraw();
        }
    };

    // Shared icon for command-line binaries (resolved once)
    std::string runIconPath = ImageView::resolve_icon_path("system-run");
    if (runIconPath.empty()) {
        runIconPath = ImageView::resolve_icon_path("utilities-terminal");
    }

    // Helper to refresh Run tab binaries (with smart ranked matching and 80-item cap)
    auto refreshRun = [&](const std::string& query) {
        const auto& binaries = BinaryManager::get_system_binaries();
        std::vector<GridItem> items;
        const size_t MAX_RESULTS = 80;
        items.reserve(MAX_RESULTS);

        std::string lower_query = query;
        std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

        if (lower_query.empty()) {
            for (size_t i = 0; i < binaries.size() && items.size() < MAX_RESULTS; ++i) {
                GridItem info;
                info.id = binaries[i];
                info.title = binaries[i];
                info.terminal = BinaryManager::is_terminal_command(binaries[i]);
                info.subtitle = info.terminal ? "Terminal Command" : "System Binary";
                info.icon_name = info.terminal ? "utilities-terminal" : "system-run";
                info.icon_path = runIconPath;
                info.exec_cmd = binaries[i];
                items.push_back(std::move(info));
            }
        } else {
            std::vector<const std::string*> prefix_matches;
            std::vector<const std::string*> contains_matches;

            for (const auto& bin : binaries) {
                std::string lower_bin = bin;
                std::transform(lower_bin.begin(), lower_bin.end(), lower_bin.begin(), ::tolower);

                if (lower_bin.rfind(lower_query, 0) == 0) {
                    prefix_matches.push_back(&bin);
                    if (prefix_matches.size() >= MAX_RESULTS) break;
                } else if (lower_bin.find(lower_query) != std::string::npos) {
                    contains_matches.push_back(&bin);
                }
            }

            for (const auto* pbin : prefix_matches) {
                if (items.size() >= MAX_RESULTS) break;
                GridItem info;
                info.id = *pbin;
                info.title = *pbin;
                info.terminal = BinaryManager::is_terminal_command(*pbin);
                info.subtitle = info.terminal ? "Terminal Command" : "System Binary";
                info.icon_name = info.terminal ? "utilities-terminal" : "system-run";
                info.icon_path = runIconPath;
                info.exec_cmd = *pbin;
                items.push_back(std::move(info));
            }

            for (const auto* pbin : contains_matches) {
                if (items.size() >= MAX_RESULTS) break;
                GridItem info;
                info.id = *pbin;
                info.title = *pbin;
                info.terminal = BinaryManager::is_terminal_command(*pbin);
                info.subtitle = info.terminal ? "Terminal Command" : "System Binary";
                info.icon_name = info.terminal ? "utilities-terminal" : "system-run";
                info.icon_path = runIconPath;
                info.exec_cmd = *pbin;
                items.push_back(std::move(info));
            }
        }

        runGrid->set_adapter(std::move(items));
        if (runGrid->get_selected_index() < 0) {
            runGrid->set_selected_index(0);
        }
        if (window) {
            window->schedule_redraw();
        }
    };

    // Listen for live window changes (open/close/focus)
    WindowManager::get()->on_windows_changed([&]() {
        refreshWindows();
    });

    // Listen for live workspace changes
    WorkspaceManager::get()->on_workspaces_changed([&]() {
        refreshWorkspaces();
    });

    // Populate initial state
    refreshWindows();
    refreshWorkspaces();

    // 6. SearchView with live filtering and enter-to-launch
    auto search = SearchViewBuilder::create()
        ->title("Apps")
        ->hint(tabHints[0])
        ->focused(true)
        ->padding(18, 12)
        ->margin(0, 0, 0, 12)
        ->onQueryTextListener(
            [&](const std::string& query) {
                if (activeTab == 0) {
                    grid->set_filter_query(query);
                } else if (activeTab == 1) {
                    windowsGrid->set_filter_query(query);
                } else if (activeTab == 2) {
                    workspacesGrid->set_filter_query(query);
                } else if (activeTab == 3) {
                    refreshRun(query);
                }
            },
            [&](const std::string& submitQuery) {
                if (activeTab == 3) {
                    if (!submitQuery.empty()) {
                        BinaryManager::launch_command(submitQuery);
                        engine->quit();
                    } else if (runGrid->get_selected_item()) {
                        PackageManager::launch(*runGrid->get_selected_item());
                        engine->quit();
                    }
                } else if (activeTab == 0) {
                    if (grid->get_selected_item()) {
                        PackageManager::launch(*grid->get_selected_item());
                        engine->quit();
                    }
                } else if (activeTab == 1) {
                    if (windowsGrid->get_selected_item()) {
                        const std::string& sel_id = windowsGrid->get_selected_item()->id;
                        auto windows = WindowManager::get()->get_windows();
                        for (auto& win : windows) {
                            if (std::to_string(win.id) == sel_id) {
                                win.activate();
                                break;
                            }
                        }
                        engine->quit();
                    }
                } else if (activeTab == 2) {
                    if (workspacesGrid->get_selected_item()) {
                        try {
                            size_t ws_id = std::stoull(workspacesGrid->get_selected_item()->id);
                            WorkspaceManager::get()->activate_workspace(ws_id);
                        } catch (...) {}
                        engine->quit();
                    }
                }
            }
        )
        ->build();

    auto switchTab = [&](int activeIdx) {
        activeTab = activeIdx;
        for (int i = 0; i < 4; ++i) {
            tabButtons[i]->set_selected(i == activeIdx);
            tabViews[i]->set_visibility(i == activeIdx ? Visibility::Visible : Visibility::Gone);
        }
        search->set_title(tabTitles[activeIdx]);
        search->set_hint(tabHints[activeIdx]);
        if (activeIdx == 1) {
            refreshWindows();
        } else if (activeIdx == 2) {
            refreshWorkspaces();
        } else if (activeIdx == 3) {
            refreshRun(search->get_query());
        }
        search->set_focused(true);
        if (window) {
            window->schedule_redraw();
        }
    };

    // 7. Horizontal Tab Bar (evenly distributes 4 tab buttons)
    auto tabBarBuilder = LinearLayoutBuilder::create()
        ->orientation(Orientation::Horizontal)
        ->spacing(8)
        ->margin(0, 0, 0, 10);

    for (int i = 0; i < 4; ++i) {
        tabButtons[i] = ButtonBuilder::create()
            ->text(tabTitles[i])
            ->bold(true)
            ->textSize(12)
            ->cornerRadius(8)
            ->padding(12, 8)
            ->onClick([i, &switchTab]() {
                switchTab(i);
            })
            ->build();
        tabBarBuilder->addView(tabButtons[i], LayoutParams(1.0f));
    }
    tabButtons[0]->set_selected(true);

    auto tabBar = tabBarBuilder->build();

    // 8. Vertical LinearLayout holding TabBar + Search + ContentStack
    auto contentLayout = LinearLayoutBuilder::create()
        ->orientation(Orientation::Vertical)
        ->addView(tabBar, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::WrapContent)))
        ->addView(search, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::WrapContent)))
        ->addView(contentStack, LayoutParams(1.0f))
        ->build();

    // 9. Modal Card Container
    auto rootCard = CardViewBuilder::create()
        ->backgroundColor(theme->colors.background)
        ->stroke(1, theme->colors.outline)
        ->cornerRadius(theme->metrics.corner_radius)
        ->padding(16)
        ->addView(contentLayout, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::MatchParent)))
        ->build();

    // 10. Window (shows immediately in <1ms without blocking on disk I/O)
    window = WindowBuilder::create()
        ->role(WindowRole::LayerOverlay)
        ->keyboardInteractive(true)
        ->dimBackdrop(true)
        ->contentSize(800, 460)
        ->contentView(rootCard)
        ->onClose([&]() {
            engine->quit();
        })
        ->onKey([&](const KeyPressEvent& event) {
            if (!event.pressed) return;
            if (event.has_shift() && event.keysym == XKB_KEY_Right) {
                switchTab((activeTab + 1) % 4);
            } else if (event.has_shift() && event.keysym == XKB_KEY_Left) {
                switchTab((activeTab + 3) % 4);
            } else if (event.has_alt()) {
                if (event.keysym >= XKB_KEY_1 && event.keysym <= XKB_KEY_4) {
                    switchTab(event.keysym - XKB_KEY_1);
                }
            }
        })
        ->build();

    if (!window) {
        return 1;
    }

    // Initial window population
    refreshWindows();

    // 11. Asynchronously scan .desktop files and populate grid on first frame
    std::thread([window, grid]() {
        auto apps = PackageManager::get_installed_applications();
        grid->set_adapter(std::move(apps));
        window->schedule_redraw();
    }).detach();

    return engine->enter_loop();
}
