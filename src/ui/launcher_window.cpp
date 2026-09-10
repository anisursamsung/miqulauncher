#include "launcher_window.hpp"
#include "grid_item_view.hpp"
#include <xkbcommon/xkbcommon-keysyms.h>

namespace miqu {

static std::vector<std::shared_ptr<View>> to_views(const std::vector<LauncherItem>& items) {
    std::vector<std::shared_ptr<View>> views;
    views.reserve(items.size());
    for (const auto& item : items) {
        views.push_back(std::make_shared<GridItemView>(item));
    }
    return views;
}

LauncherWindow::LauncherWindow(AppEngine* engine, LauncherConfig config)
    : m_engine(engine), m_config(std::move(config)), m_active_mode_index(m_config.active_mode_index) {}

bool LauncherWindow::init() {
    if (!m_engine || m_config.modes.empty()) return false;

    if (m_active_mode_index >= m_config.modes.size()) {
        m_active_mode_index = 0;
    }

    if (m_config.dmenu_mode) {
        m_dmenu_provider.load_from_stdin();
    }

    auto config = Config::get();

    // 1. GridView setup
    m_grid = GridViewBuilder::create()
        ->autoFit(m_config.cell_size)
        ->cellHeight(m_config.cell_size)
        ->spacing(m_config.spacing, m_config.spacing)
        ->onItemClick([this](size_t index, std::shared_ptr<View> view) {
            handle_item_click(index, view);
        })
        ->build();

    const auto& init_mode = m_config.modes[m_active_mode_index];

    // 2. Search View (Mode indicator in title pill + search input)
    m_search = SearchViewBuilder::create()
        ->title(init_mode.display_label)
        ->hint(init_mode.hint)
        ->focused(true)
        ->padding(18, 12)
        ->margin(0, 0, 0, 12)
        ->onQueryTextListener(
            [this](const std::string& query) {
                refresh_current_mode();
            },
            [this](const std::string& submitQuery) {
                handle_submit(submitQuery);
            }
        )
        ->build();

    if (!m_config.initial_query.empty()) {
        m_search->set_query(m_config.initial_query);
    }

    // 3. Vertical content layout (Clean: only Search Bar + Grid)
    auto contentLayout = LinearLayoutBuilder::create()
        ->orientation(Orientation::Vertical)
        ->addView(m_search, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::WrapContent)))
        ->addView(m_grid, LayoutParams(1.0f))
        ->build();

    // 4. Modal Card container
    auto rootCard = CardViewBuilder::create()
        ->backgroundColor(config->colors.background)
        ->stroke(config->metrics.border_width, config->colors.outline)
        ->cornerRadius(config->metrics.corner_radius)
        ->padding(16)
        ->addView(contentLayout, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::MatchParent)))
        ->build();

    // 5. Layer overlay Window
    auto builder = WindowBuilder::create()
        ->role(WindowRole::LayerOverlay)
        ->appId("miqulauncher")
        ->keyboardInteractive(true)
        ->preferredSize(m_config.width, m_config.height)
        ->contentSize(m_config.width, m_config.height)
        ->closeOnClickOutside(true)
        ->closeOnEscape(true)
        ->contentView(rootCard)
        ->onClose([this]() {
            m_engine->quit();
        });

    if (!m_config.dim_backdrop) {
        builder->anchors(0)->dimBackdrop(false);
    } else {
        builder->dimBackdrop(true);
    }

    m_window = builder
        ->onKey([this](const KeyPressEvent& event) {
            if (!event.pressed) return;
            if (m_config.allow_mode_switch && m_config.modes.size() > 1) {
                if (event.has_shift() && event.keysym == XKB_KEY_Right) {
                    switch_mode((m_active_mode_index + 1) % m_config.modes.size());
                } else if (event.has_shift() && event.keysym == XKB_KEY_Left) {
                    switch_mode((m_active_mode_index + m_config.modes.size() - 1) % m_config.modes.size());
                } else if (event.has_alt()) {
                    if (event.keysym >= XKB_KEY_1 && event.keysym < XKB_KEY_1 + static_cast<uint32_t>(m_config.modes.size())) {
                        switch_mode(event.keysym - XKB_KEY_1);
                    }
                }
            }
        })
        ->build();

    if (!m_window) {
        return false;
    }

    // Live state subscriptions
    m_window_provider.on_windows_changed([this]() {
        if (m_config.modes[m_active_mode_index].type == ModeType::Window) {
            refresh_current_mode();
        }
    });

    m_workspace_provider.on_workspaces_changed([this]() {
        if (m_config.modes[m_active_mode_index].type == ModeType::Workspace) {
            refresh_current_mode();
        }
    });

    // Populate initial mode
    switch_mode(m_active_mode_index);

    // Pre-scan desktop apps in the background
    m_app_provider.load_async([this]() {
        if (m_config.modes[m_active_mode_index].type == ModeType::App) {
            refresh_current_mode();
        }
    });

    return true;
}

void LauncherWindow::switch_mode(size_t mode_index) {
    if (mode_index >= m_config.modes.size()) return;
    m_active_mode_index = mode_index;
    const auto& mode = m_config.modes[mode_index];

    if (mode.type == ModeType::Script) {
        m_script_provider = std::make_unique<ScriptProvider>(mode.script_path);
    } else {
        m_script_provider.reset();
    }

    m_search->set_title(mode.display_label);
    m_search->set_hint(mode.hint);
    m_search->set_focused(true);

    refresh_current_mode();
}

void LauncherWindow::refresh_current_mode() {
    std::string query = m_search ? m_search->get_query() : "";
    const auto& mode = m_config.modes[m_active_mode_index];

    switch (mode.type) {
        case ModeType::App: {
            auto items = m_app_provider.get_items(query);
            m_grid->set_items(to_views(items));
            break;
        }
        case ModeType::Window: {
            auto items = m_window_provider.get_items(query);
            m_grid->set_items(to_views(items));
            break;
        }
        case ModeType::Workspace: {
            int active_idx = -1;
            auto items = m_workspace_provider.get_items(query, &active_idx);
            m_grid->set_items(to_views(items));
            if (active_idx >= 0 && query.empty()) {
                m_grid->set_selected_index(active_idx);
            }
            break;
        }
        case ModeType::Run: {
            auto items = m_run_provider.get_items(query);
            m_grid->set_items(to_views(items));
            if (m_grid->get_selected_index() < 0 && !items.empty()) {
                m_grid->set_selected_index(0);
            }
            break;
        }
        case ModeType::Script: {
            if (m_script_provider) {
                auto items = m_script_provider->query(query);
                m_grid->set_items(to_views(items));
                if (m_grid->get_selected_index() < 0 && !items.empty()) {
                    m_grid->set_selected_index(0);
                }
            }
            break;
        }
        case ModeType::Dmenu: {
            auto items = m_dmenu_provider.query(query);
            m_grid->set_items(to_views(items));
            if (m_grid->get_selected_index() < 0 && !items.empty()) {
                m_grid->set_selected_index(0);
            }
            break;
        }
    }

    if (m_window) {
        m_window->schedule_redraw();
    }
}

void LauncherWindow::handle_item_click(size_t index, std::shared_ptr<View> view) {
    auto item_view = std::dynamic_pointer_cast<GridItemView>(view);
    if (!item_view) return;

    const auto& data = item_view->get_data();
    const auto& mode = m_config.modes[m_active_mode_index];

    switch (mode.type) {
        case ModeType::App:
            m_app_provider.launch(data);
            break;
        case ModeType::Window:
            m_window_provider.activate(data);
            break;
        case ModeType::Workspace:
            m_workspace_provider.activate(data);
            break;
        case ModeType::Run:
            m_run_provider.launch(data);
            break;
        case ModeType::Script:
            if (m_script_provider) {
                m_script_provider->activate(data);
            }
            break;
        case ModeType::Dmenu:
            m_dmenu_provider.activate(data);
            break;
    }

    m_engine->quit();
}

void LauncherWindow::handle_submit(const std::string& query) {
    const auto& mode = m_config.modes[m_active_mode_index];

    if (mode.type == ModeType::Run && !query.empty()) {
        m_run_provider.launch_raw(query);
        m_engine->quit();
        return;
    }

    if (mode.type == ModeType::Dmenu) {
        if (auto item = m_grid->get_selected_item()) {
            handle_item_click(m_grid->get_selected_index(), item);
        } else if (!query.empty()) {
            m_dmenu_provider.submit(query);
            m_engine->quit();
        }
        return;
    }

    if (auto item = m_grid->get_selected_item()) {
        handle_item_click(m_grid->get_selected_index(), item);
    }
}

} // namespace miqu
