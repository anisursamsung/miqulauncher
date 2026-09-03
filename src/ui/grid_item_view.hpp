#pragma once

#include "model/launcher_item.hpp"
#include <miqutoolkit/view/view.hpp>
#include <memory>

namespace miqu {

class GridItemView : public View {
public:
    explicit GridItemView(LauncherItem data);
    ~GridItemView() override = default;

    void draw(cairo_t* cr, const Rect& bounds) override;

    const LauncherItem& get_data() const { return m_data; }
    LauncherItem& get_data() { return m_data; }
    void set_data(LauncherItem data) { m_data = std::move(data); }

private:
    LauncherItem m_data;
};

} // namespace miqu
