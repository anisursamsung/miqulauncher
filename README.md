# 🚀 Miqulauncher

A lightning-fast, native Wayland application menu launcher built with **Miqutoolkit** and **wlr-layer-shell**.

---

## ✨ Features

- **Instant Startup (<1ms):** Launches immediately and asynchronously scans `.desktop` entries in the background on a detached thread without freezing the UI.
- **Adaptive Auto-Fit Grid:** Dynamically calculates grid columns based on window dimensions and screen resolution.
- **Live Search Filtering:** Instant fuzzy/substring searching across application names, executable commands, categories, and descriptions.
- **Keyboard & Mouse Navigation:**
  - Full arrow key navigation (<kbd>↑</kbd> <kbd>↓</kbd> <kbd>←</kbd> <kbd>→</kbd>), <kbd>PageUp</kbd>, and <kbd>PageDown</kbd>.
  - Launch selected item with <kbd>Enter</kbd>.
  - Quick dismiss with <kbd>Escape</kbd> or by clicking outside the modal.
- **Dynamic Theming Integration:** Automatically syncs with **Miquland** theme configurations (`theme_mode.conf`, `dark.conf`, `light.conf`) with zero manual setup.
- **Overlay & Backdrop Dimming:** Modern semi-transparent backdrop blur and darkened overlay.

---

## 📦 Dependencies & Installation

### 1. Requirements
- **`miqutoolkit`** (must be installed first)
- **`wayland`**, **`wayland-protocols`**, **`cairo`**, **`pango`**

### 2. Build & Install to `/usr/bin/miqulauncher`
```bash
cd miqulauncher
sudo ./make.sh
```

---

## ⌨️ Usage

### Run from Terminal
```bash
miqulauncher
```

### Keybinding in `miquland.conf`
Add the following line to `~/.config/miquland/miquland.conf`:
```ini
bind = Super+Space, miqulauncher
```

### Keybinding in Hyprland / Sway
- **Hyprland:**
  ```ini
  bind = SUPER, SPACE, exec, miqulauncher
  ```
- **Sway:**
  ```ini
  bindsym $mod+space exec miqulauncher
  ```

---

## ⚙️ Theming

Miqulauncher automatically inherits your active theme from `~/.config/miquland/theme/theme_mode.conf`:
- When set to `dark.conf`, Miqulauncher uses the Material Neon Dark palette.
- When set to `light.conf`, Miqulauncher dynamically switches to the Material Light palette.

---

## 📜 License

MIT License. Developed as part of the Miquland Desktop Ecosystem.
