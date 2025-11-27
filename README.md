# hyprpew

A Hyprland plugin that adds smooth **curtain-style closing animations** for windows. When you close a window, it smoothly shrinks from both sides toward the center, like curtains closing on a stage.

## Features

- Smooth curtain-close animation when windows are closed
- Configurable animation duration
- Subtle edge highlights for a polished look
- Compatible with hyprpm for easy installation

## Installation

### Using hyprpm (Recommended)

```bash
hyprpm add https://github.com/purple-lines/hyprpew
hyprpm enable hyprpew
hyprpm reload
```

### Manual Installation

1. Make sure you have the Hyprland development headers installed
2. Clone this repository
3. Build the plugin:

```bash
make all
```

4. Load the plugin in your `hyprland.conf`:

```conf
plugin = /path/to/hyprpew.so
```

Or use hyprctl:

```bash
hyprctl plugin load /path/to/hyprpew.so
```

## Configuration

Add these options to your `hyprland.conf`:

```conf
plugin {
    hyprpew {
        # Animation duration in seconds (default: 0.4)
        duration = 0.4
    }
}
```

## How It Works

The plugin hooks into Hyprland's window close event and renders a curtain-style overlay animation:

1. When a window closes, the plugin captures its position and size
2. Dark curtain overlays are drawn from both the left and right edges
3. The curtains smoothly move toward the center using an ease-out exponential curve
4. Subtle white edge highlights add depth to the effect
5. The animation completes as the curtains meet in the middle

## Dependencies

- Hyprland (latest version recommended)
- pkg-config
- Standard C++ build tools (g++ or clang++)

## Building from Source

```bash
# Clone the repository
git clone https://github.com/purple-lines/hyprpew.git
cd hyprpew

# Build
make all

# The plugin will be built as hyprpew.so
```

## Troubleshooting

### Plugin fails to load with version mismatch

This means the plugin was compiled against different Hyprland headers than your running Hyprland version. Rebuild the plugin:

```bash
make clean
make all
```

### Animation not showing

Make sure the plugin is properly loaded:

```bash
hyprctl plugin list
```

You should see `hyprpew` in the list.
