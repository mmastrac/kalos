# Language Guide

Kalos is primarily designed as a scripting language to be integrated into an existing application
for customization and event-handling purposes.

The language is heavily inspired by both C-like languages and Python. 

## The basics

Kalos has first-class support for event handlers as a way for the host program to trigger actions
in the script.

Most applications will provide a built-in init function like so, and trigger it immediately after
the application launches and is ready for interaction:

```C
on init {
    println("Hello, world!");
}
```

As with most other languages, Kalos supports local and global variables and constants:

```C
var global = "hello";
const global_const = "world";

on init {
    const comma = ", ";
    # hello, world
    println(global + comma + world);
}
```

### Control structures

The most basic control structures in Kalos are `if` and `loop`. Unlike most C-like languages,
Kalos does not require parenthesis in `if` statements:

```C
if condition {
    println("true!");
} else {
    println("false.");
}
```

Kalos does not provide conditional loops, but instead a generic `loop` function along with `break`
and `continue` statements to terminate the loop:

```C
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

### Iterators and for loops

In addition to the generic `loop`, Kalos provides a `for` that allows for iteration over various iterable
objects.

Kalos provides a number of built-in iterable objects including lists, ranges and string splitters.

```C
for x in [1, 2, 3] {
    # ...
}
```

```C
for s in split("a,b,c", ",") {
    # ...
}
```

```C
for i in range(0, 20) {
    # ...
}
```

All control structures in Kalos can be nested, and any looping structures support `break` and `continue`
as you would expect:

```C
for i in range(0, 20) {
    if i == 10 && !flag {
        break;
    }
}
```

### Expressions

Expressions in Kalos make use of the common operators you see in other C-like languages. The precedence, or
the "order of operations" in Kalos is designed to be more human-friendly as you might find in languages like
Python.

| Operators | Notes |
|-----------|-------|
| `()`      | Parenthetized expressions |
| `x[index]`, `x(...)`, `x.attr` | Indexing expressions, function calls, object attributes |
| Unary `+`, `-`, `!`, `~` | Unary operators: posivate, negate, logical NOT, bitwise NOT |
| `*`, `/`, `%` | Multiplication, division, modulus |
| `+`, `-` | Plus, minus |
| `<<`, `>>` | Left and right shift |
| `&` | Bitwise AND |
| `^` | Bitwise XOR |
| `\|` | Bitwise OR |
| `<`, `>`, `<=`, `>=` | Less than, greater than, less than or equal, greater than or equal |
| `==`, `!=` | Equal, not equal |
| `&&` | Logical AND |
| `\|\|` | Logical OR |
| `+=`, `-=`, ... | Assignment operators: any binary operator is supported |

In Kalos, the logical AND and OR have the lowest precedence, allowing you to perform bit tests
without paretheses (although they are still recommended for readability):

```C
# Equivalent to: (bitmask & flag1) || (bitmask & flag2)
if bitmask & flag1 || bitmask & flag2 {
    # ...
}
```

### Functions

Kalos supports both "binding" and "internal" functions, which use `on` and `fn` respectively. Internal
functions are useful to de-duplicate logic between multiple handlers.

```C
fn create_output(id, name) {
    return "{id}: {name}";
}

on app.keyup {
    println(create_output(123, "foo"));
}

on app.keydown {
    println(create_output(456, "bar"));
}
```

## Integration

Kalos is integrated into your application using bindings declared in a `kidl` file. The KIDL describes the
modules, functions, objects and properties that your module provides.

The KIDL consists of a top-level `idl` block which contains nested, named `module` blocks. Kalos supports the
addition of functions to the root (aka builtin) namespace by creating a module named `builtin`.

### Dispatch generator

Kalos ships with a cross-platform KIDL compiler that you can use to optionally compile your KIDL to a binary format for
more efficient embedding, and to compile dispatch glue code to allow for bidirectional communication with the
embedded scripts.

Pre-compilation of scripts and KIDL is optional, but at this time the dispatch glue is required for integration.

### Handlers

Handlers are the main way for host applications to communicate with scripts. A handler is defined in a module using
the `handler` keyword. Any parameters that the handler receives may be specified here as well and must match those
in the script receiving the event.

```C
idl {
    module app {
        handler keyup(key: number);
        handler keydown(key: number);
    }
}
```

The script embedded in the application would handle these events using the `on` keyword:

```C
on app.keyup(key: number) {
    # ...
}
```

When generating the bindings, Kalos will create a trigger method for each `handler` that the host app 
can call with the appropriate parameters.

```C
void kalos_module_app_trigger_app_keyup(kalos_run_state* state, kalos_int number);

// ...
int last_key = readkey();
kalos_module_app_trigger_app_keydown(state, last_key);
while (keydown()) {}
kalos_module_app_trigger_app_keyup(state, last_key);
```

### Module functions

TODO

### Module properties

TODO

### Bindings

TODO

### Objects

TODO
