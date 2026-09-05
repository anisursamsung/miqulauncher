# Miqulauncher

A fast Wayland layer-shell application launcher built with Miqutoolkit.

## Features

- **Asynchronous App Loading**: Scans `.desktop` entries in the background without blocking the UI.
- **Search & Filter**: Real-time filtering by application name, category, or executable command.
- **Modes**: Supports Application, Window, and Workspace modes.
- **Navigation**: Full keyboard (arrows, Enter, Escape) and mouse support.
- **Theming**: Automatically syncs colors and font/rounding metrics from `miquland.conf`.

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
