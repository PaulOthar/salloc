# makes - Convenience make files

*makes* is a collection of convenient `make` files.
This collection is initially designed for C code, but may eventually support other languages/tools.

# Using a script

You may add a script to your project makefile by downloading this project and referencing it in your main `Makefile`:

```makefile
include makes/util.mk
```

# Functions

Functions in make can be called in the following way:

```makefile
$(call function_name,param_1,param_2,...)
```

it is preferred that you put no space between the commas, to avoid errors during execution, since it may add the space to the final expression.

# Files

|Name|Description|
|-|-|
|[util.mk](doc/util.md)|Utility functions for other files|
|[valgrind.mk](doc/valgrind.md)|Valgrind and Callgrind convenience functions|
|[release.mk](doc/release.md)|Functions for compiling libraries and managing dependencies|