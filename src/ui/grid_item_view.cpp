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

    auto config = Config::get();

    // Draw App Icon
    Rect icon_rect(bounds.x + (bounds.width - 48) / 2, bounds.y + 12, 48, 48);
    ImageView icon_view(m_data.icon_path.empty() ? m_data.icon_name : m_data.icon_path);
    icon_view.set_target_size(48);
    icon_view.draw(cr, icon_rect);

    // Draw App Title
    PangoLayout* layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, m_data.title.c_str(), -1);

    std::string font_family = config->metrics.font_family.empty() ? "Sans" : config->metrics.font_family;
    int font_size = config->metrics.font_size > 0 ? config->metrics.font_size : 10;
    std::string font_spec = font_family + " " + std::to_string(font_size);

    PangoFontDescription* desc = pango_font_description_from_string(font_spec.c_str());
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);

    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
    pango_layout_set_width(layout, std::max(0, bounds.width - 8) * PANGO_SCALE);
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);

    cairo_move_to(cr, bounds.x + 4, bounds.y + 66);
    cairo_set_source_rgba(cr, config->colors.on_surface.r,
                              config->colors.on_surface.g,
                              config->colors.on_surface.b,
                              config->colors.on_surface.a);
    pango_cairo_show_layout(cr, layout);
    g_object_unref(layout);

    // Draw Subtitle if present
    if (!m_data.subtitle.empty()) {
        PangoLayout* sub_layout = pango_cairo_create_layout(cr);
        pango_layout_set_text(sub_layout, m_data.subtitle.c_str(), -1);

        int sub_size = std::max(6, font_size - 2);
        std::string sub_font_spec = font_family + " " + std::to_string(sub_size);

        PangoFontDescription* sub_desc = pango_font_description_from_string(sub_font_spec.c_str());
        pango_layout_set_font_description(sub_layout, sub_desc);
        pango_font_description_free(sub_desc);

        pango_layout_set_alignment(sub_layout, PANGO_ALIGN_CENTER);
        pango_layout_set_width(sub_layout, std::max(0, bounds.width - 8) * PANGO_SCALE);
        pango_layout_set_ellipsize(sub_layout, PANGO_ELLIPSIZE_END);

        cairo_move_to(cr, bounds.x + 4, bounds.y + 80);
        if (m_data.subtitle.find("Active") != std::string::npos) {
            cairo_set_source_rgba(cr, config->colors.primary.r,
                                      config->colors.primary.g,
                                      config->colors.primary.b,
                                      1.0f);
        } else {
            cairo_set_source_rgba(cr, config->colors.on_surface_variant.r,
                                      config->colors.on_surface_variant.g,
                                      config->colors.on_surface_variant.b,
                                      0.75f);
        }
        pango_cairo_show_layout(cr, sub_layout);
        g_object_unref(sub_layout);
    }
}

} // namespace miqu
