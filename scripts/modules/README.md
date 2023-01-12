# Application Extension Modules
This folder contains modules that can be `require`d by the application to provoce enhanced capabilities or streamline the application script.

- [Application Extension Modules](#application-extension-modules)
  - [Including frameworks and libraries](#including-frameworks-and-libraries)
  - [Application Framework (app)](#application-framework-app)
    - [Using the framework in your Script](#using-the-framework-in-your-script)
    - [Application Framework API](#application-framework-api)
      - [Applet Values](#applet-values)
      - [app.opts](#appopts)
      - [app.new](#appnew)
      - [A.framework\_version](#aframework_version)
      - [A.framework\_license](#aframework_license)
      - [A:help](#ahelp)
      - [A:show\_version](#ashow_version)
      - [A:go](#ago)
      - [A:message](#amessage)
      - [A:show\_bin](#ashow_bin)
- [Base64 Conversion Tools](#base64-conversion-tools)
- [Progress Bar](#progress-bar)
  - [Description](#description)
  - [Progress Bar API](#progress-bar-api)
    - [Including the Progress Bar in you applet](#including-the-progress-bar-in-you-applet)
    - [progress.version](#progressversion)
    - [progress.size](#progresssize)
    - [progress.min](#progressmin)
    - [progress.max](#progressmax)
    - [progress.step](#progressstep)
    - [progress.position](#progressposition)
    - [progress.char\[\]](#progresschar)
    - [progress.bar](#progressbar)
    - [progress.new](#progressnew)
    - [PB:pos](#pbpos)
    - [PB:next](#pbnext)
    - [PB:render](#pbrender)
- [Acknowledgements](#acknowledgements)

## Including frameworks and libraries

When using the modules in this directory, Lua will not know to search for them in the `./modules` directory.  To enable this to function correctly, you will need to add the directory to the Lua package path.

```Lua
-- ---------------------------------------------------------------------------
local sep  = package.config:sub(1,1) -- extract the separator
package.path = ("..{SEP}modules{SEP}?.lua;.{SEP}modules{SEP}?.lua;"):gsub("{SEP}",sep) .. package.path
------------------------------------------------------------------------------
```

## Application Framework (app)

**file**: modules/app.lua

The application framework is a simple wrapper for applications that processes and organizes command line arguments, creates output files, and handles some other aspects of applications such as application messages.

### Using the framework in your Script

To include the framework in your application use a require statement at the top of the application script and create a new meta-table object of the application by calling the `app.new()` function.  The application framework calls 2 callback functions that contain the main entry point of the applet.  See the API section for more information about the framework objects.

```Lua
-- A Real simple application using the framework -----------------------------
local app = require "app"  -- include application framework
local my_app = app.new()   -- create the META-table object for your application
-- Application initialization ------------------------------------------------
function my_app:init()
    -- Set application default options, check some things or perform any
    -- initialization for the application that is required.
end

-- Application Main ----------------------------------------------------------
function my_app:main(ifiles, ofile)
   -- ifiles is a table containing the input files from the command line
   -- ofile is a Lua FILE object that was opened for writing (text mode)

   return true
end
------------------------------------------------------------------------------
my_app:go(arg)
return my_app
```

### Application Framework API


#### Applet Values

Once created, the applet framework has default values generated for several values that are used by the framework.  The following table describes the applet values that can be used.

| Applet Value |         Type         | Description                                                                                                                           |
| :----------: | :------------------: | :------------------------------------------------------------------------------------------------------------------------------------ |
|    `name`    |       `string`       | The name of the applet command.  This is usually the executable file name                                                             |
|   `breif`    |       `string`       | A brief, typically 1 line, description of the applet function                                                                         |
|  `version`   | `string` or `number` | The version identifier for the applet.  Typically, `string` values are used to support semantic versioning of the applet.             |
|   `detail`   |       `string`       | A long, detailed description of the applet, command line options and other information to assist the user when new to the applet use. |
|  `license`   |       `string`       | A string that contains the license type and information for the applet                                                                |
| ` copyright` |       `string`       | The copyright date and other information regarding the copyright information for the applet                                           |

#### app.opts

The application command line arguments are parsed into flags and assignments, then stored into the `app.opts` table.  This table uses the option name as the index, and the value of the option as the value stored in the table index.

For example `--ofile=this.txt --option` would produce a table containing the assignments:

```Lua
app.opts = {
  ofile = "this.txt",
  option = true
}
```

> ![Note](../../img/note50x50.png) All command line options are stored in the opts table, there are some, such as `ofile`, `quiet`, `version`, and `help` are handled directly by the framework and have special meaning.

#### app.new

```Lua
A = app.new( bin_mode, plain )
```

|  Argument  | Supported<br/>Types | Description                                                                  | Default |
| :--------: | :-----------------: | :--------------------------------------------------------------------------- | :-----: |
| `bin_mode` |      `boolean`      | A value that identifies the output file as binary (`true`) or text (`false`) | `false` |
|  `plain`   |      `boolean`      | A value that when true will disable the ANSI color codes to the output.      | `false` |

This function is called to create an applet framework meta-table object in the Lua applet script.  It should be called at the start of the script to create the applet object.  When created, this function will return the applet object `A`.

```Lua
-- Example object creation
local app = require "app"          -- require the applet framework
local my_applet = app.new()        -- use the new() function to create the object
```

#### A.framework_version

```Lua
str = A.framework_version()
```

This function returns the version of the applet framework as a Lua string value

#### A.framework_license

```Lua
str = A.framework_license()
```

This function returns the licensing information of the framework.

#### A:help

```Lua
A:help()
```

The `A:help()` function will display the detailed help information for the applet.  Use the `A.detail` value to define the output of the help function.  This function is called automatically by the framework when the `--help` option is passed on the command line.

#### A:show_version

```Lua
A:show_version()
```

This function is called automatically by the framework when the `--version` option is passed on the command line.  By default, it will print the string "{name}: v{version}" on the console, where the {name} is the value of `A.name` and the value of the {version} tag is `A.version`.

#### A:go

```Lua
A:go( args, no_ansi )
```

| Argument  | Supported<br/>Types  | Description                                                                                                                                                                                                                                                                                                                                                                    | Default |
| :-------: | :------------------: | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :-----: |
|  `args`   | `table` of `strings` | This argument is the command line data that has been processed as a table by Lua.  Normally this is the value `arg` that is defined by the Lua execution environment and contains a parsed command line (the argc,argv) data passed to the 'C' main() function.  This value can be overridden for testing or additional arguments can be passed to set arguments when required |  `arg`  |
| `no_ansi` |      `boolean`       | This argument is used to disable ANSI escape sequence outputs from the framework.  All escape sequences are filtered out and plaintext is output.                                                                                                                                                                                                                              | `false` |

#### A:message

```Lua
A:message( state, message, ... )
```

| Argument  | Supported<br/>Types | Description                                                          | Default  |
| :-------: | :-----------------: | :------------------------------------------------------------------- | :------: |
|  `state`  |      `string`       | A string defining the applet state.                                  | `"info"` |
| `message` |      `string`       | A format string used to define the message to be displayed           |   `""`   |
|   `...`   |       various       | Additional values to be substituted in placeholders of the `message` |  `nil`   |

`A:message()` is a function that is used to output message strings to the console.  This function uses the format `[%s]: %s` to generate a format string from the `state` and `message`.  Once the format string is built, `string.format()` is used to build the output message that is displayed.  The `state` argument is pre-processed by the `A:message()` function to change the color of the output string in the field used for the `state`.

| State     | color  |
| --------- | ------ |
| `"info"`  | Green  |
| `"warn"`  | Yellow |
| `"error"` | Red    |

> ![Note](../img/../../img/note50x50.png) Other state strings are not processed and when not including a color string (`{c#}`), the state sill display in Green (`"{c10}"`).

#### A:show_bin

```Lua
str = A:show_bin(pkt, line_length, bin_only)
```

|   Argument    | Supported<br/>Types | Description                                                                             | Default |
| :-----------: | :-----------------: | :-------------------------------------------------------------------------------------- | :-----: |
|     `pkt`     |      `string`       | This is a string that contains the binary data to be displayed                          |  `nil`  |
| `line_length` |      `number`       | An integer value that contains the number of bytes to be displayed on one line.         |  `16`   |
|  `bin_only`   |      `boolean`      | A flag used to suppress the output of the ASCII output to the right of the binary data. | `false` |

This function is used to output a block of binary data as hexadecimal values as well as an optional ASCII representation of the printable ASCII characters.  The binary data to display is passed as the `pkt` argument.  The optional `line_length` argument is used to specify the number of bytes displayed per line of output and the `bin_only` flag is used to suppress the output of the ASCII data when set to `true`.  A `false` or `nil` value of `bin_only` will append the ASCII Data to the end of the hexadecimal output.

```Lua
-- Example Output
[00][01][02][03][04]  .....
```

---

# Base64 Conversion Tools

---

# Progress Bar

An ASCII progress bar for showing progress or other information

> ![Warning](../../img/alert48x48.png) There are two versions of the progress bar library included.  It is recommended that `progress2.lua` be used for all new applets.  `progress.lua` has been deprecated and is not recommended for use.

## Description

The progress bar library is designed as a modular library to simplify progress bar creation to show operation progress, and/or other information that has a min and max value (somewhat) graphically using ASCII.  The progress bar is rendered into a string that can be used with all other string functions.  The minimum value, maximum value and overall length (in characters) of the bar can be set to customize the output strings rendered.  The bar will also track the present position and provides an update function that both updates the bar to the next step, and also renders the updated progress bar string.

> ![Note](../../img/note50x50.png) The rendered string may contain escape sequences intended for the `ansi()` output function.  This can be overridden by setting the strings used when rendering the progress bar.


## Progress Bar API

The progress bar application programming interface (API) information assumes the variable name `PB` for progress bar meta-table objects, and uses the `progress` variable when referring to the data returned from the `require` statement.

### Including the Progress Bar in you applet

To use the progress bar object, `require` the progress bar Lua script at the start of your applet then use the `progress.new()` function to generate a new meta-table object to be used in the applet.

```Lua
progress = require "progress2"  -- use Progress2 rather than progress.  
PB = progress.new()  -- create the new progress bar object (using defaults)
```

### progress.version

This object value is the present version of the progress bar implementation.  Since this value can be overridden by the applet object, it is recommended to always use `progress.version` when checking the progress bar library version.

### progress.size

The `size` of the progress bar is the total number of characters used to render the bar horizontally.  This value can be overridden by assigning a value to `PB.size` in the applet object.

### progress.min

The minimum numerical value of the value range handled by the progress bar.

### progress.max

The maximum numerical value of the value range handled by the progress bar.

### progress.step

The individual step size of the progress bar when updating to the `next()` value.  This is the value added to the present position of the bar when updating.

### progress.position

The present position of the progress bar.  Normally this is not directly accessed, rather is handled by calling the `PB:pos()` function to read or write the present position.

### progress.char[]

This value is a table of strings used when rendering the bar.  There are 5 values in the indexed table that are 0%, 25%, 50%, 75%, and 100% of each individual character comprising the bar.  For example, if there was a bar that was 10 characters long that represented a value of 0 through 100, each step would represent 10 units.  The progress bar would render a value of 1 through 2.5 as 25% character, 2.6 through 5 as the 50% character and so on until 10 was achieved at which point the 100% character would be used for that position in the bar.  The characters can be overridden from their default value of `progress.char = {"{c7}=", "{c4}\\", "{c12}|", "{c14}/", "{c10}#" }` to use any ASCII characters desired for the applet.

### progress.bar
The `.bar` is a string that contains the format string for the output progress bar.  This string contains the placeholder tags `${BAR}` to represent the progress bar content. 

### progress.new

```Lua
PB = progress.new(size, max, min)
```

| Argument | Supported<br/>Types | Description                                                            | Default |
| :------: | :-----------------: | :--------------------------------------------------------------------- | :-----: |
|  `size`  |      `number`       | This value represents the number of characters used to render the bar. |   50    |
|  `max`   |      `number`       | The maximum value that can be represented by the bar.                  |   100   |
|  `min`   |      `number`       | The minimum value that can be represented by the bar.                  |    0    |

The `progress.new()` function is used to create a new meta-table object that is the applet progress bar.  This implementation enables multiple progress bars to be generated and maintained by an applet with unique ranges and customizations as required within the applet.  The default configuration of the progress bar can be overridden through assigning values to the arguments passed to the function.  The `size` argument is used to set the overall length of the progress bar in characters.  When it is not specified, the progress bar is assumed to be 50 characters long.  The range covered by the progress bar can be specified by assigning values to the `max` and `min` arguments.  When not assigned, the values of `100` and `0` are assumed respectively.  The function will return an initialized meta-table object for the progress bar.

### PB:pos

```Lua
current, str = PB:pos( new )
```

| Argument | Supported<br/>Types | Description                                                                     | Default |
| :------: | :-----------------: | :------------------------------------------------------------------------------ | :-----: |
|  `new`   |      `number`       | Optional argument that will assign a new value to the bar position when present |  `nil`  |

This function is used to set or get the present position tracked by the progress bar.  Additionally, this function will also render the present progress bar to a string and return the rendered string as a second return value.  When the `new` argument is assigned a value, the current position of the bar will be updated to the value stored in `new`.

### PB:next

```Lua
str = PB:next()
```

The `PB:next()` function will update the current position to the next step, and then execute a render of the updated progress to a string that is returned.

### PB:render

```Lua
str = PB:render()
```

This function will render the progress bar to a string.  The rendered string is returned.

---

**Author**: Chuck Erhardt<br>
**Revision**: 0.5.0<br>
**Date**: 27-DEC-2022<br>
(c)2021-2022 E2ForLife.com, CC-BY-SA-NC v4.0

| <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">![Creative Commons License](../../img/CC88x31.png)</a> | This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>. |
| -------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |

---

# Acknowledgements

+ [Note](https://icons8.com) icon by [Icons8](https://icons8.com)
+ [Alert](https://icons8.com) icon by [Icons8](https://icons8.com)