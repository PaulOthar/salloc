# release.mk

Some useful functions for compiling libraries.

## build_release

Generates an *archive* file with the specified name, based on current project source code.
The generated files will be stored in the local directory `release`, and the *archive* file will be stored in `./release/lib`.

It takes into account only the `./src` directory, so any file outside of this directory will not be included.

```makefile
$(call build_release,lib_name)
```

## build_dependency

For each subdirectory in the specified directory, it will attempt to run `make release` command.

```makefile
$(call build_dependency,directory)
```