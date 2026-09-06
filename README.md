# Miqulauncher

A fast Wayland layer-shell application launcher built with Miqutoolkit.

## Features

- **Asynchronous App Loading**: Scans `.desktop` entries in the background without blocking the UI.
- **Search & Filter**: Real-time filtering by application name, category, or executable command.
- **Modes**: Supports Application, Window, and Workspace modes.
- **Navigation**: Full keyboard (arrows, Enter, Escape) and mouse support.
- **Theming & Configuration**: Fully independent customization via `~/.config/miqulauncher/miqulauncher.conf`.

## Configuration

Configuration is loaded from `~/.config/miqulauncher/miqulauncher.conf` (falls back to `/usr/share/miqulauncher/miqulauncher.conf`):

```ini
# Window & Grid Layout
width = 800
height = 460
cell_size = 100
spacing = 10

# Colors (Hex: #RRGGBB or #RRGGBBAA)
background = #f4f8fc
surface = #ffffff
surface_variant = #e6eff8
primary = #0066ff
on_primary = #ffffff
text = #0f172a
text_muted = #475569
outline = #99c2ff

# Styling & Metrics
corner_radius = 12
border_width = 1
font = Sans
font_size = 11
icon_theme = Papirus
```

## Dependencies

- `miqutoolkit` (must be installed to `/usr`)
- `wayland`, `wayland-protocols`, `cairo`, `pango`

## Build & Install

```bash
cd miqulauncher
# Build locally
./make.sh

# Install system-wide to /usr/bin/miqulauncher
sudo ./make.sh
```

## Keybinding Setup

Add to `~/.config/miquland/miquland.conf`:
```ini
bind = Super+Space, miqulauncher
```

Or for Hyprland:
```ini
bind = SUPER, SPACE, exec, miqulauncher
```
