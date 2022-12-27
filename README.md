# Lua Applet Compiler

A Lua based applet generator for command line tools using Lua.

## Overview

This is an applet framework that I was working on to develop a Lua based build system for embedded applications.  Since the applet framework has become more of a generic application development system for Lua based applications, I split it out of the build system and created a repository for the tools and code.

## Why use Lua for this?

While there are many great things about developing command-line tools using typical compiled languages such as Python, Rust, C/C++ (and many others), Lua offers a few things that help to make applet construction quick and powerful.

1. Lua is very light-weight, and the whole language is compiled right with the code that is being run.  That means that the applet can load and parse Lua scripts like data files to set parameters or provide more advanced input that the command line would normally provide.  It also means that there is less work including an interpreter in the applet since it's already there.
2. Lua has advanced string processing as a basic part of the language.
3. The scripts can be deployed as scripts for testing, then compiled for deployment without changing anything.  The separate environment that runs the script is the same one that is compiled into the applet.

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

| <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">![Creative Commons License](../../img/CC88x31.png)</a> | This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>. |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |

---

# Acknowledgements

+ [Note](https://icons8.com) icon by [Icons8](https://icons8.com)
+ [Alert](https://icons8.com) icon by [Icons8](https://icons8.com)
