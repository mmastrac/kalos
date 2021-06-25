# Language Guide

Kalos is primarily designed as a scripting language to be integrated into an existing application
for customization and event-handling purposes.

The language is heavily inspired by both C-like languages and Python. 

## The basics

Kalos has first-class support for event handlers as a way for the host program to trigger actions
in the script.

Most applications will provide a built-in init function like so, and trigger it immediately after
the application launches and is ready for interaction:

```
on init {
    println("Hello, world!");
}
```

As with most other languages, Kalos supports local and global variables and constants:

```
var global = "hello";
const global_const = "world";

on init {
    const comma = ", ";
    # hello, world
    println(global + comma + world);
}
```

## Control structures

The most basic control structures in Kalos are `if` and `loop`. Unlike most C-like languages,
Kalos does not require parenthesis in `if` statements:

```
if condition {
    println("true!");
} else {
    println("false.");
}
```

Kalos does not provide conditional loops, but instead a generic `loop` function along with `break`
and `continue` statements to terminate the loop:

```
loop {
    if readkey() == key.ESCAPE {
        break;
    } else if (readkey() == key.ENTER) {
        app.select();
        continue;
    }
    println("Unknown key pressed");
}
```

## Iterators and for loops

In addition to the generic `loop`, Kalos provides a `for` that allows for iteration over various iterable
objects.

Kalos provides a number of built-in iterable objects including lists, ranges and string splitters.

```
for x in [1, 2, 3] {
    # ...
}
```

```
for s in split("a,b,c", ",") {
    # ...
}
```

```
for i in range(0, 20) {
    # ...
}
```

All control structures in Kalos can be nested, and any looping structures support `break` and `continue`
as you would expect:

```
for i in range(0, 20) {
    if i == 10 && !flag {
        break;
    }
}
```
