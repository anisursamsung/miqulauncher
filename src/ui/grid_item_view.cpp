#include "grid_item_view.hpp"

namespace miqu {

GridItemView::GridItemView(LauncherItem data)
    : TileView(data.title, data.icon_path.empty() ? data.icon_name : data.icon_path, data.is_image)
    , m_data(std::move(data)) {
    set_subtitle(m_data.subtitle);
    set_highlight_subtitle(m_data.subtitle.find("Active") != std::string::npos);
    if (m_data.is_image) {
        set_quality_mode(ImageQuality::ThumbnailFast);
    }
}

void GridItemView::set_data(LauncherItem data) {
    m_data = std::move(data);
    set_title(m_data.title);
    set_icon_source(m_data.icon_path.empty() ? m_data.icon_name : m_data.icon_path);
    set_is_image(m_data.is_image);
    set_subtitle(m_data.subtitle);
    set_highlight_subtitle(m_data.subtitle.find("Active") != std::string::npos);
    if (m_data.is_image) {
        set_quality_mode(ImageQuality::ThumbnailFast);
    }
}

} // namespace miqu
