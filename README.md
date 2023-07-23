# Virtual Input Panel

Author: Shukai Ni

## Build and Install

```shell
qmake -v
kf5-config --version
```

```shell
cmake -B build && cmake --build build
cmake --install build
```

```shell
sudo chown root:root build/vip
sudo chmod 4755 build/vip
```

## Keyboard JSON Representation Documentation

The following is a detailed documentation for the JSON representation of a keyboard layout. This representation is designed to capture the structure and attributes of a keyboard, including keys with different sizes and weights.

- totalWidth (integer): The total width of the keyboard in pixels.

- totalHeight (integer): The total height of the keyboard in pixels.

- spacing (double): The spacing between keys. It represents the gap between adjacent keys in pixels.

- color (string): The color of the key labels and symbols represented in hexadecimal format (e.g., "#000000" for black).

- background (string): The background color of the keyboard represented in hexadecimal format (e.g., "#ffffff" for white).

- rows (array): An array of objects representing individual rows on the keyboard.

  - rowSpan (double): The row span weight compared to other rows. It determines the height of the row relative to the total keyboard height, with default weight being 1 if not specified.

  - keys (array): An array of objects representing individual keys within the row.

    - type (string): The type of the key. This can include values like "key," "mouse_button," "modifier," "special," etc. This allows for future extension to include different types of keys with specific behaviors.

    - code (integer or string): The key code or identifier for the key. This could be either an integer (e.g., 1, 2, 16, etc.) or a string (e.g., "KEY_ESC," "KEY_Q," etc.) based on convenience and readability. If using integer codes, it is recommended to document the mapping between the integers and their respective meanings.

    - label (string, optional): The label displayed on the key. It provides a human-readable description for the key. For example, "Esc" for the escape key.

    - columnSpan (double, optional): The column span weight compared to other keys in the row. It determines the width of the key relative to other keys in the same row, with default weight being 1 if not specified. If a key needs to occupy more than one column width, specify a higher weight for that key.

This JSON representation aims to provide a comprehensive and flexible way to describe keyboard layouts while supporting easy modifications and extensions in the future. It allows for capturing various keyboard configurations, key types, and weights to accommodate different key sizes and layouts in modern keyboard designs.

## TODO

- [ ] Touch / button input
- [ ] Support for input macro
- [x] Exit key
- [ ] Multi touch event to support multiple touch
