# Kalos: The tiny embeddable scripting system

[![CI Build](https://github.com/mmastrac/kalos/actions/workflows/build.yml/badge.svg)](https://github.com/mmastrac/kalos/actions/workflows/build.yml)

Kalos is a very lightweight, easy-to-integrate scripting system for applications wanting to add
easy runtime configuration and automation.

Kalos is heavily inspired by Python, but offers a more C-like syntax.

Kalos runs on everything from Arduinos to low-memory 8086 DOS all the way up to 64-bit machines!

```
import app;
import color;
import window;

on app.init {
    var x;
    app.fill(0, 0, window.WIDTH, window.HEIGHT, color.BLUE);
    # Draw lines every four pixels
    for x in range(0, window.WIDTH / 4) {
        app.drawline(x * 4, 0, x * 4, window.HEIGHT, color.LIGHTBLUE);
    }
    app.messagebox("Hello, world!");
}
```

## Features 

 * Tiny VM with low requirements
 * Allocation-free parser for parsing script on-device
 * Binary script format for pre- or external compilation in memory-constrained applications
 * Comprehensive control structures: `if`/`else`, `for` over iterators, `loop`, `break`/`continue`
 * IDL-based bindings to your code
 * Basic list support
 * String interpolation

Script documentation coming soon.

## Getting Started

Coming soon.
