# Modern Title Bar with Win32 API

This project is a C demonstration of creating a modern, custom-drawn title bar for Windows applications using the raw Win32 API. It demonstrates how to replace the standard Windows title bar with a minimalist design, complete with custom-drawn menus and caption buttons, all while integrating modern Windows 11 UI features like rounded corners.

## How does it works?

- Handles `WM_CREATE` and calculates the `rect` for each part of the menu
- Draws the title bar on top of the original title bar in `WM_PAINT`
- Draws everything else and handle owner-drawn menu
- Uses Windows 11's `DwmSetWindowAttribute` to remove the border of the menu
and add rounded corner
- Caption buttons is drawn with the Windows 11's standard Segoe UI font library

## Preview

Here’s a look at the custom title bar and the overall window style.

**Custom Title Bar**

![Title Bar](/images/Titlebar.png)

**Full Window**

![Window](/images/Window.png)

**Rounded submenu**

![Submenu](/images/Submenu.png)

## Building the Project

To build and run this demo, you'll need the MSVC compiler toolset and CMake.

You can run the provided **very simple** batch script. It just build the project and run it:

```bash
.\build.bat
```

If you prefer building it manually, just build it like any CMake project:

```bash
cmake -B build
cd build
cmake --build .
```

After building, you must link the manifest file to enable DPI scaling:

```bash
mt.exe -manifest ../assets/manifest.xml -outputresource:.\Debug\demo.exe
```

You can now find and run demo.exe inside the build\Debug directory.

## Known Issues & Future Plans

This project is a proof-of-concept and has a few limitations:
- **Missing Submenu Support:** The current menu implementation does not yet draw or handle nested submenus. The next major goal is to implement this feature.
- **Multi-Monitor Dragging:** Dragging the window seamlessly between displays with different DPI settings is not fully supported and can be buggy.
- **Minimum Size Constraint:** The window currently cannot be resized to a width smaller than the combined size of the menu items and the caption buttons. We plan to add logic to handle this gracefully, perhaps with an overflow menu.
