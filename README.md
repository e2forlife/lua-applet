# Lua Applet Compiler

A Lua based applet generator for command line tools using Lua.

## Overview

This is an applet framework that I was working on to develop a Lua based build system for embedded applications.  Since the applet framework has become more of a generic application development system for Lua based applications, I split it out of the build system and created a repository for the tools and code.

## Why use Lua for this?

While there are many great things about developing command-line tools using typical compiled languages such as Python, Rust, C/C++ (and many others), Lua offers a few things that help to make applet construction quick and powerful.

1. Lua is very light-weight, and the whole language is compiled right with the code that is being run.  That means that the applet can load and parse Lua scripts like data files to set parameters or provide more advanced input that the command line would normally provide.  It also means that there is less work including an interpreter in the applet since it's already there.
2. Lua has advanced string processing as a basic part of the language.
3. The framework adds Lua extensions that include custom functions for processing data, collecting input, and other common operations.
4. The scripts can be deployed as scripts for testing, then compiled for deployment without changing anything.  The separate environment that runs the script is the same one that is compiled into the applet.

## The Lua Environment

This framework is presently based on the Lua version 5.4 language syntax, and include several enhancements for working with data and conversion to/from sized types (such as `uint32` and `uint16`). There are also some enhancements made for handling user input to facilitate simpler applet implementation.

### Number to string conversion

```Lua
str = uint64( num, big )
num = uint64( str, big )

str = uint32( num, big )
num = uint32( str, big )

str = uint16( num, big )
num = uint16( str, big )

str = float( num, big )
num = float( str, big )

str = double( num, big )
num = double( str, big )
```

| Argument | Supported<br/>Types | Description                                          | Default |
| :------: | :-----------------: | :--------------------------------------------------- | :-----: |
|  `num`   |      `number`       | A number value to be converted to a string           |  `nil`  |
|  `str`   |      `string`       | A string value to be converted to a number           |  `nil`  |
|  `big`   |      `boolean`      | A boolean value used to enable big-endian conversion | `false` |

These functions are used to convert a number to a string or a string to an integer with a fixed number of sized bits.  The argument `big` can be set to `true` to enable big-endian conversion between strings and numbers.

> ![Note](img/note50x50.png) To convert 8-bit values use the string.byte() and string.char() functions.

### delay

```Lua
delay( ms )
```

| Argument | Supported<br/>Types | Description                         | Default |
| :------: | :-----------------: | :---------------------------------- | :-----: |
|   `ms`   |      `number`       | The number of milliseconds to delay |    1    |

The `delay()` function is a Lua extension to allow a script to wait for a number of milliseconds passed as the `ms` argument.  While waiting, **no code will execute**.

### ansi

```Lua
ansi( val )
```

| Argument | Supported<br/>Types | Description             | Default |
| :------: | :-----------------: | :---------------------- | :-----: |
|  `val`   |         any         | The value to be printed |  `""`   |

`ansi()` is an alternate output function to `print()` that will send a string to `stdout`, however will not append the newline (`\n`) character to the end of the output.  Additionally, `ansi()` can parse escape sequences to insert ANSI VT100 command codes into the strings.

Escape sequences are semicolon (`;`) separated command sequences that are bracketed with curly-braces (`{}`).  An example command `{c5}` would change the output color to VT100 color 5, or purple.

| Command | Description                                     |
| :-----: | :---------------------------------------------- |
|   `c`   | Change the foreground color of output           |
|   `b`   | Change the background color of output           |
|  `up`   | Move the cursor up on the display               |
| `down`  | Move the cursor down on the display             |
| `left`  | move the cursor left                            |
| `right` | move the cursor right                           |
|  `mv`   | move the cursor to a specific row, col position |
| `hide`  | hide the cursor                                 |
| `show`  | display the cursor                              |
|  `csr`  | Clear the screen                                |
|  `clr`  | clear the current line                          |

The 

### getc

```Lua
str = getc()
```

`getc()` is an input function that will return a 4 byte scan code that is the scan code for the key that was pressed.  This function does not block, and will return `nil` when no key was pressed.  Use ![`kbhit()`](#kbhit) to check for a key-press before calling `getc()` when there is a need to know the status of the key before reading the pressed key value.

### kbhit

```Lua
bool = kbhit()
```

The `kbhit()` function is used to check for a key-press without removing the key from the input buffer.  When there is a key present, `kbhit()` will return a value of `true`, otherwise a value of `false` is returned.  This function is non-blocking, rather than waiting for a period of time to check, this function will return immediately the status of the input buffer.

## Installation and Usage

To install the framework, clone the repository to a local directory for your applet, then build the environment, followed by the tools.  Once the environment and tools are compiled, you can start development of the applet.

### Building the Lua command environment

When building the Lua environment, compile from VSCode with the `build_flags = -DLUA_EXE` line uncommented.  Once compiled, copy the `.pio/native/build/program` to the `bin` folder, and rename it to `xLua.exe`.

### Building the Lua Compiler

Once the Lua environment is build, run the `scripts\build.bat` batch file to use the Lua compiler to compile itself. This batch file will run the compiler to generate C code from the Lua source, then move the generated C output source to the output source directory.  Edit the `platformio.ini` file to uncomment the `build_flags = -DLCOMPILE` line and comment all other `build_flags=` lines.  Run the PlatoformIO compile operation, then copy the output `.pio/native/build/program.exe` to the `bin` folder, and rename it to `LCompile.exe`.


### Build Steps for your applet

1. Use `lcompile` to convert the Lua scripts for your applet to C code.
2. Copy the output `.c` file to the `src` directory
3. Edit the `platformio.ini` file to add the #define setup when compiling the Lua source for the applet.
4. Build the application using the PlatformIO build command.
5. Move the `.pio/native/build/program` output to the `bin` folder and rename appropriately.

---

**Author**: Chuck Erhardt<br>
**Revision**: 0.5.0<br>
**Date**: 27-DEC-2022<br>
(c)2021-2022 E2ForLife.com, CC-BY-SA-NC v4.0

| <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">![Creative Commons License](./img/CC88x31.png)</a> | This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>. |
| -------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |

---

# Acknowledgements

+ [Note](https://icons8.com) icon by [Icons8](https://icons8.com)
+ [Alert](https://icons8.com) icon by [Icons8](https://icons8.com)
