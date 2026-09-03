#include "grid_item_view.hpp"
#include <miqutoolkit/view/image_view.hpp>
#include <miqutoolkit/core/color_scheme.hpp>
#include <pango/pangocairo.h>
#include <algorithm>

namespace miqu {

GridItemView::GridItemView(LauncherItem data)
    : m_data(std::move(data)) {}

void GridItemView::draw(cairo_t* cr, const Rect& bounds) {
    if (!cr || bounds.width <= 0 || bounds.height <= 0) return;

    auto theme = ColorScheme::get();

    // Draw App Icon
    Rect icon_rect(bounds.x + (bounds.width - 48) / 2, bounds.y + 12, 48, 48);
    ImageView icon_view(m_data.icon_path.empty() ? m_data.icon_name : m_data.icon_path);
    icon_view.set_target_size(48);
    icon_view.draw(cr, icon_rect);

    // Draw App Title
    PangoLayout* layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, m_data.title.c_str(), -1);

    PangoFontDescription* desc = pango_font_description_from_string("Sans 10");
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);

    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
    pango_layout_set_width(layout, std::max(0, bounds.width - 8) * PANGO_SCALE);
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);

    cairo_move_to(cr, bounds.x + 4, bounds.y + 66);
    cairo_set_source_rgba(cr, theme->colors.on_surface.r,
                              theme->colors.on_surface.g,
                              theme->colors.on_surface.b,
                              theme->colors.on_surface.a);
    pango_cairo_show_layout(cr, layout);
    g_object_unref(layout);

    // Draw Subtitle if present
    if (!m_data.subtitle.empty()) {
        PangoLayout* sub_layout = pango_cairo_create_layout(cr);
        pango_layout_set_text(sub_layout, m_data.subtitle.c_str(), -1);

        PangoFontDescription* sub_desc = pango_font_description_from_string("Sans 8");
        pango_layout_set_font_description(sub_layout, sub_desc);
        pango_font_description_free(sub_desc);

        pango_layout_set_alignment(sub_layout, PANGO_ALIGN_CENTER);
        pango_layout_set_width(sub_layout, std::max(0, bounds.width - 8) * PANGO_SCALE);
        pango_layout_set_ellipsize(sub_layout, PANGO_ELLIPSIZE_END);

        cairo_move_to(cr, bounds.x + 4, bounds.y + 80);
        if (m_data.subtitle.find("Active") != std::string::npos) {
            cairo_set_source_rgba(cr, theme->colors.primary.r,
                                      theme->colors.primary.g,
                                      theme->colors.primary.b,
                                      1.0f);
        } else {
            cairo_set_source_rgba(cr, theme->colors.on_surface_variant.r,
                                      theme->colors.on_surface_variant.g,
                                      theme->colors.on_surface_variant.b,
                                      0.75f);
        }
        pango_cairo_show_layout(cr, sub_layout);
        g_object_unref(sub_layout);
    }
}

} // namespace miqu
