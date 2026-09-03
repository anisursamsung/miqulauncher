#include "launcher_window.hpp"
#include "grid_item_view.hpp"
#include <xkbcommon/xkbcommon-keysyms.h>

namespace miqu {

static const char* s_tab_titles[4] = { "Apps", "Windows", "Workspaces", "Run" };
static const char* s_tab_hints[4] = {
    "Search applications...",
    "Switch to open window...",
    "Switch or filter workspace...",
    "Run command line or binary..."
};

static std::vector<std::shared_ptr<View>> to_views(const std::vector<LauncherItem>& items) {
    std::vector<std::shared_ptr<View>> views;
    views.reserve(items.size());
    for (const auto& item : items) {
        views.push_back(std::make_shared<GridItemView>(item));
    }
    return views;
}

LauncherWindow::LauncherWindow(AppEngine* engine)
    : m_engine(engine) {}

bool LauncherWindow::init() {
    if (!m_engine) return false;

    auto theme = ColorScheme::get();

    // 1. GridView setup
    m_grid = GridViewBuilder::create()
        ->autoFit(100)
        ->cellHeight(100)
        ->spacing(10, 10)
        ->onItemClick([this](size_t index, std::shared_ptr<View> view) {
            handle_item_click(index, view);
        })
        ->build();

    // 2. Tab Buttons Bar
    auto tabBarBuilder = LinearLayoutBuilder::create()
        ->orientation(Orientation::Horizontal)
        ->spacing(8)
        ->margin(0, 0, 0, 10);

    for (int i = 0; i < 4; ++i) {
        m_tab_buttons[i] = ButtonBuilder::create()
            ->text(s_tab_titles[i])
            ->bold(true)
            ->textSize(12)
            ->cornerRadius(8)
            ->padding(12, 8)
            ->onClick([this, i]() {
                switch_tab(i);
            })
            ->build();
        tabBarBuilder->addView(m_tab_buttons[i], LayoutParams(1.0f));
    }
    m_tab_buttons[0]->set_selected(true);
    auto tabBar = tabBarBuilder->build();

    // 3. Search View
    m_search = SearchViewBuilder::create()
        ->title("Apps")
        ->hint(s_tab_hints[0])
        ->focused(true)
        ->padding(18, 12)
        ->margin(0, 0, 0, 12)
        ->onQueryTextListener(
            [this](const std::string& query) {
                refresh_current_tab();
            },
            [this](const std::string& submitQuery) {
                handle_submit(submitQuery);
            }
        )
        ->build();

    // 4. Vertical content layout (TabBar + Search + Grid)
    auto contentLayout = LinearLayoutBuilder::create()
        ->orientation(Orientation::Vertical)
        ->addView(tabBar, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::WrapContent)))
        ->addView(m_search, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::WrapContent)))
        ->addView(m_grid, LayoutParams(1.0f))
        ->build();

    // 5. Modal Card container
    auto rootCard = CardViewBuilder::create()
        ->backgroundColor(theme->colors.background)
        ->stroke(1, theme->colors.outline)
        ->cornerRadius(theme->metrics.corner_radius)
        ->padding(16)
        ->addView(contentLayout, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::MatchParent)))
        ->build();

    // 6. Layer overlay Window
    m_window = WindowBuilder::create()
        ->role(WindowRole::LayerOverlay)
        ->keyboardInteractive(true)
        ->dimBackdrop(true)
        ->closeOnClickOutside(true)
        ->closeOnEscape(true)
        ->contentSize(800, 460)
        ->contentView(rootCard)
        ->onClose([this]() {
            m_engine->quit();
        })
        ->onKey([this](const KeyPressEvent& event) {
            if (!event.pressed) return;
            if (event.has_shift() && event.keysym == XKB_KEY_Right) {
                switch_tab((m_active_tab + 1) % 4);
            } else if (event.has_shift() && event.keysym == XKB_KEY_Left) {
                switch_tab((m_active_tab + 3) % 4);
            } else if (event.has_alt()) {
                if (event.keysym >= XKB_KEY_1 && event.keysym <= XKB_KEY_4) {
                    switch_tab(event.keysym - XKB_KEY_1);
                }
            }
        })
        ->build();

    if (!m_window) {
        return false;
    }

    // Live state subscriptions
    m_window_provider.on_windows_changed([this]() {
        if (m_active_tab == 1) {
            refresh_current_tab();
        }
    });

    m_workspace_provider.on_workspaces_changed([this]() {
        if (m_active_tab == 2) {
            refresh_current_tab();
        }
    });

    // Populate Apps tab and scan .desktop asynchronously
    refresh_current_tab();
    m_app_provider.load_async([this]() {
        if (m_active_tab == 0) {
            refresh_current_tab();
        }
    });

    return true;
}

void LauncherWindow::switch_tab(int tab_index) {
    if (tab_index < 0 || tab_index >= 4) return;
    m_active_tab = tab_index;

    for (int i = 0; i < 4; ++i) {
        m_tab_buttons[i]->set_selected(i == tab_index);
    }

    m_search->set_title(s_tab_titles[tab_index]);
    m_search->set_hint(s_tab_hints[tab_index]);
    m_search->set_focused(true);

    refresh_current_tab();
}

void LauncherWindow::refresh_current_tab() {
    std::string query = m_search ? m_search->get_query() : "";

    if (m_active_tab == 0) {
        auto items = m_app_provider.get_items(query);
        m_grid->set_items(to_views(items));
    } else if (m_active_tab == 1) {
        auto items = m_window_provider.get_items(query);
        m_grid->set_items(to_views(items));
    } else if (m_active_tab == 2) {
        int active_idx = -1;
        auto items = m_workspace_provider.get_items(query, &active_idx);
        m_grid->set_items(to_views(items));
        if (active_idx >= 0 && query.empty()) {
            m_grid->set_selected_index(active_idx);
        }
    } else if (m_active_tab == 3) {
        auto items = m_run_provider.get_items(query);
        m_grid->set_items(to_views(items));
        if (m_grid->get_selected_index() < 0 && !items.empty()) {
            m_grid->set_selected_index(0);
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
    if (m_active_tab == 0) {
        m_app_provider.launch(data);
    } else if (m_active_tab == 1) {
        m_window_provider.activate(data);
    } else if (m_active_tab == 2) {
        m_workspace_provider.activate(data);
    } else if (m_active_tab == 3) {
        m_run_provider.launch(data);
    }

    m_engine->quit();
}

void LauncherWindow::handle_submit(const std::string& query) {
    if (m_active_tab == 3) {
        if (!query.empty()) {
            m_run_provider.launch_raw(query);
            m_engine->quit();
            return;
        }
    }

    if (auto item = m_grid->get_selected_item()) {
        handle_item_click(m_grid->get_selected_index(), item);
    }
}

} // namespace miqu
