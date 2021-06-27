# Kalos: The tiny embeddable scripting system

[![CI Build](https://github.com/mmastrac/kalos/actions/workflows/build.yml/badge.svg)](https://github.com/mmastrac/kalos/actions/workflows/build.yml)

<img align="right" src="docs/kalos-logo-ebo.png" style="float: right; height: 200px;">

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
    app.messagebox("Message from {app.title}", "Hello, world!");
}
```

## Features 

 * Tiny VM with low system resource requirements
 * Minimal C requirements: any C99-like compiler (`avr-gcc` and `openwatcom` are both supported)
 * Allocation-free parser and custom allocator support for parsing and running scripts on resource-constrained devices
 * Pluggable string system to build your own to support extended character sets or memory requirements
 * Binary script format for pre- or external compilation in constrained applications
 * Comprehensive control structures: `if`/`else`, `for` over iterators, `loop`, `break`/`continue`
 * IDL-based bindings to your code with dispatch code generator
 * Basic list support (improvements coming in later revisions)
 * String interpolation by default
 * Varargs-style invocation

## Limitations

 * The string system currently assumes ASCII, though it will work with UTF-8 in some cases (some functions
 may not work properly in the presence of multi-byte, extended UTF-8 characters).
 * Lists are currently immutable and the design for mutable lists/list builders is not complete.
 * Dictionaries and/or maps are not implemented.
 * Objects may not be defined by script code, and object functions are not available.

## Getting Started

Read the [Language Guide](docs/LanguageGuide.md) for tips on getting started. Check out the [DOS](example/dos) and
[Arduino/AVR](example/avr) samples for examples of how you might get started.
