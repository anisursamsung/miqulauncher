#include <miqutoolkit/miqutoolkit.hpp>
#include <thread>

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
        ->onItemClick([&](const AppInfo& app) {
            PackageManager::launch(app);
            engine->quit();
        })
        ->build();

    // 2. SearchView with left title label & live filtering
    auto search = SearchViewBuilder::create()
        ->title("Search")
        ->hint("Type to filter applications...")
        ->focused(true)
        ->padding(18, 12)
        ->margin(0, 0, 0, 12)
        ->onQueryTextListener([grid](const std::string& query) {
            grid->set_filter_query(query);
        })
        ->build();

    // 3. Vertical LinearLayout holding Search + Grid
    auto contentLayout = LinearLayoutBuilder::create()
        ->orientation(Orientation::Vertical)
        ->addView(search, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::WrapContent)))
        ->addView(grid, LayoutParams(1.0f))
        ->build();

    // 4. Modal Card Container
    auto rootCard = CardViewBuilder::create()
        ->backgroundColor(theme->colors.background)
        ->stroke(1, theme->colors.outline)
        ->cornerRadius(theme->metrics.corner_radius)
        ->padding(16)
        ->addView(contentLayout, LayoutParams(static_cast<int>(LayoutDimension::MatchParent), static_cast<int>(LayoutDimension::MatchParent)))
        ->build();

    // 5. Window (shows immediately in <1ms without blocking on disk I/O)
    auto window = WindowBuilder::create()
        ->role(WindowRole::LayerOverlay)
        ->keyboardInteractive(true)
        ->dimBackdrop(true)
        ->contentSize(800, 420)
        ->contentView(rootCard)
        ->onClose([&]() {
            engine->quit();
        })
        ->build();

    if (!window) {
        return 1;
    }

    // 6. Asynchronously scan .desktop files and populate grid on first frame
    std::thread([window, grid]() {
        auto apps = PackageManager::get_installed_applications();
        grid->set_adapter(std::move(apps));
        window->schedule_redraw();
    }).detach();

    return engine->enter_loop();
}
