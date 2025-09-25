# DLauncher

![Version](https://img.shields.io/badge/version-0.3-blue)
![License](https://img.shields.io/badge/license-GPLv3-blue)
![Platform](https://img.shields.io/badge/platform-linux--x86%7Carm-lightgrey)
![Language](https://img.shields.io/badge/language-C%2B%2B17-blue)
![Last Commit](https://img.shields.io/github/last-commit/dayvster/DLauncher)

DLauncher is a fast, minimal, and highly customizable Qt-based application launcher for Linux. It lets you search and launch your installed apps with style and speed.

---

## Features

- Super fast fuzzy search for all your desktop apps
- Fully themable via `~/.config/dlauncher/theme` (colors, fonts, window size/position, padding, border radius, and more)
- Customizable window geometry (width, height, position) and UI padding
- Font and font size selection for a personalized look
- Deduplication and prioritization of user-installed apps over system apps
- Robust .desktop file parsing and normalization
- Launches any app with all its arguments and flags (handles complex Exec fields)
- Modern, minimal, distraction-free UI
- Cross-platform build with CMake (Linux, BSD, and more)
- Lightweight, fast startup and low memory usage
- Simple C++17/Qt codebase, easy to hack and extend
- No telemetry, no ads, no nonsense EVER

---



## Quick Start

```bash
git clone https://github.com/dayvster/DLauncher.git
cd DLauncher
./build.sh
./build/DLauncher
```

Or build manually:

```bash
mkdir build && cd build
cmake ..
make
../build/DLauncher
```

---

## Configuration & Theming

DLauncher is fully customizable via a simple config file:

**`~/.config/dlauncher/theme`**

```ini
# Example theme
background=#222222
border=#444444
text=#ffffff
highlight=#00bcd4
input_bg=#222222
input_border=#444444
font=FiraCode
font_size=18
border_radius=12
padding=14
window_width=600
window_height=350
window_pos_x=100
window_pos_y=100
selection=#00bcd4cc
row_bg=#33333322
row_hover=#00bcd455
```

If the file is missing or empty, DLauncher uses beautiful sane defaults.

---

## Dependencies

- Qt 5 or 6 (for GUI)
- C++17 compiler
- CMake 3.10+

---

## License

This project is licensed under the [GPLv3](LICENSE).

---

## Contributing

PRs and issues are welcome! If you have ideas for features, theming, or want to help, just open an issue or pull request.

### Prerequisites


## Dependencies 

### Building

Clone the repository:

```bash
git clone https://github.com/dayvster/DLauncher.git
cd DLauncher
```

## Build using the provided script

```shell
./build.sh
```

## Or manually using CMake:

```shell

mkdir build
cd build
cmake ..
make
```
