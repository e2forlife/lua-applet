# Lua to C Compiler
Compile Lua application to C code and autogenerate loader

## Overview
The Lua-to-C compiler is used to convert the Lua scripts (and the dependencies) to C code which contains loadable modules that are used by the application wrapper.

## Build commands

To build the source from the Lua script for the compiler that is used for the Lua scripts, run the following commands from the shell prompt.

```sh
# Windows users: Use the \ to replace the / and replace mv with the DOS equalivant (move) command/.
cd $PROJECT_HOME$/scripts/utilities  
../xLua compiler.lua --ofile=compiler.c --def=LCOMPILE --app=compiler ../modules/app.lua ../modules/progress.lua ./compiler.lua

mv --force compiler.c ../../src/compiler.c
```

## Usage
Lcompile is a Lua compiler that was written in Lua!  It allows for the compilation
of Lua source files into compiled binary modules and/or a C source file that can
be included in a standalone executable project.
```sh
Lcompile --quiet --def=<define> --app=<object> --obj --ofile=<output> ...
```

|  Option  | Description                                          |
| :------: | ---------------------------------------------------- |
| --quiet  | Suppress messages on the console.                    |
| --ofile= | define the target C-source file.                     |
|  --def=  | Include #define flags around generate source         |
|  --app=  | module name of the application                       |
|  --obj   | generate binary object modules for each source input |
|   ...    | the input Lua file to compile to binary              |

### Version 2.1+ note

**Version 2.1.0** (and up) of lcompile modifies the default file generation algorithm for faster response.  This can be modified using the `--fast` option flag on the command line to accelerate the output by not generating newlines in the output data blocks.  This can greatly accelerate the file processing time required to generate the chunk files, however, the files will not be very readable.  When compatibility is required with previious version of lcompile, the `--legacy` option can be used to enable compatibility mode.

New options and features

+ New fast generation of compiled source files
+ `--fast` flag added to accelerate further by dropping line splits and processing files as one block
+ `--legacy` flag add to enable *compatibility mode* with v1.0 lcompile
+ `--obj` flag new allows the assignment of a target path for the storage of compiled output
+ compiler detects pre-compiled source (extensions that are not `lua`) and inserts rather than load and compile.

### Description

   Lcompile will load the input specified Lua files and compile them to a
   binary chunk creating a source file containing a loader and an execution
   function stubs for running the code.  Code that is included with `require`
   statements in the main application is compiled and stored in the output
   C-source file.  
   
   A note on object generation:
   This version of lcompile enable the pre-compilation of object files into
   *.chunk.c output objects.  You can use the --obj= option to set the object
   directory path, or just use the --obj to specify the current directory as
   the object path.

   The order of the input files on the command line are important
   for the generated C-source, as they are loaded in the order that they are
   specified.  So, make sure that interdependencies are handled by placing
   files that require other files LAST in the list.  For example if there
   was a project that contains a common code that is used by both the
   main applet scripts and some other support function scripts, place
   the common code used by both files before the modules that use them.  The
   compiler does not (at this time) support resolution of dependencies.
   
   To use this tool, pass the main source to the compiler as an input file
   along with the source of all the required modules (unless you want to
   package the Lua with the exe).  All dependencies that are "required" are
   loaded and included in the output binary chunk when specified in the build
   list, otherwise they will be required from Lua and the source or Lua chunk
   must be accessible on the `package.path`.

----

# Lua based binary encoder
Encode binary files as Lua source for inclusion as a requirement or as a datafile that is read by the Lua environment.

## Build Commands

```sh
cd $PROJECT_HOME$/scripts/utilities
../xLua compiler.lua --ofile=encbin.c --def=ENCBIN --app=encbin ../modules/app.lua ../modules/progress.lua ../modules/base64.lua ./encbin.lua
mv --force encbin.c ../../src/encbin.c
```