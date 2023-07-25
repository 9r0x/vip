# Virtual Input Panel

Author: Shukai Ni

![Demo](Demo.png)

## Build and Install

```shell
yay -S extra-cmake-modules
kf5-config --version
```

```shell
cmake -B build && cmake --build build
cmake --install build
sudo chown root:root build/vip
sudo chmod 4755 build/vip
```

## Why Virtual Input Panel is more than on-screen keyboard(OSK)

1. Touchscreen support
2. Flexible configuration for layout, style, and input devices(keyboard, mouse, etc.)
3. Kernel level input event support
4. Near real-keyboard experience: modifier keys work like real keyboard
5. Orthogonal to input methods and system hotkeys

## Notes

1. Mouse events are not enabled by default and discouraged, because a simulated mouse event would create a event loop without careful layout design.
2. Right ctrl and right alt is by default not included because 1) to save layout space so that other keys have sufficient size 2) it is hard to press right ctrl and right alt on a touchscreen

## Keyboard JSON Representation Documentation

The following is a detailed documentation for the JSON representation of a keyboard layout. This representation is designed to capture the structure and attributes of a keyboard, including keys with different sizes and weights.

- totalWidth (integer): The total width of the keyboard in pixels.

- totalHeight (integer): The total height of the keyboard in pixels.

- spacing (double): The spacing between keys. It represents the gap between adjacent keys in pixels.

- style (string): The CSS style for all keys. It is a string that can be used to specify the default style of all keys in the keyboard. For example, "background-color: #ffffff; color: #000000".

- rows (array): An array of objects representing individual rows on the keyboard.

  - rowSpan (double): The row span weight compared to other rows. It determines the height of the row relative to the total keyboard height, with default weight being 1 if not specified.

  - keys (array): An array of objects representing individual keys within the row.

    - type (string): The type of the key. This can include values like "key," "mouse_button," "modifier," "special," etc. This allows for future extension to include different types of keys with specific behaviors.

    - code (integer): The key code or identifier for the key. This could be an integer (e.g., 1, 2, 16, etc.) from `/usr/include/linux/input-event-codes.h`.

    - label (string, optional): The label displayed on the key. It provides a human-readable description for the key. For example, "Esc" for the escape key.

    - columnSpan (double, optional): The column span weight compared to other keys in the row. It determines the width of the key relative to other keys in the same row, with default weight being 1 if not specified. If a key needs to occupy more than one column width, specify a higher weight for that key.

    - style (string, optional): The CSS style for the key. It is a string that can be used to specify the style of the key. For example, "background-color: #ffffff; color: #000000".

This JSON representation aims to provide a comprehensive and flexible way to describe keyboard layouts while supporting easy modifications and extensions in the future. It allows for capturing various keyboard configurations, key types, and weights to accommodate different key sizes and layouts in modern keyboard designs.

## TODO

- [x] Exit key
- [x] Multi touch event to support multiple touch
- [x] Support either click / touch event
- [x] Custom theme
- [x] Special key for panel reposition
- [x] Placeholder that reserves space for custom keys
- [x] Custom keys
- [ ] Mouse emulation
- [ ] Haptic feedback(sound, effects?)
- [ ] Support for input macro
- [ ] Layout switch
- [ ] Spacing config
- [ ] Improve reposition smoothness
- [ ] Special key for panel zoom
- [ ] System level config at /etc/vip.d/\*.json
