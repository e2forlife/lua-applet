# Application Extension Modules
This folder contains modules that can be `require`d by the application to provoce enhanced capabilities or streamline the application script.

- [Application Extension Modules](#application-extension-modules)
  - [Including frameworks and libraries](#including-frameworks-and-libraries)
  - [Application Framework (app)](#application-framework-app)
    - [Using the framework in your Script](#using-the-framework-in-your-script)
    - [Application Framework API](#application-framework-api)
      - [Applet Values](#applet-values)
      - [app.new](#appnew)
      - [A.framework\_version](#aframework_version)
      - [A.framework\_license](#aframework_license)
      - [A:help](#ahelp)
      - [A:show\_version](#ashow_version)
      - [A:go](#ago)
      - [A:message](#amessage)
      - [A:show\_bin](#ashow_bin)
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