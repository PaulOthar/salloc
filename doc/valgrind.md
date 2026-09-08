# valgrind.mk

Some useful Valgrind functions. LINUX EXCLUSIVE

## valprof

Runs Callgrind on the specified binary.
The generated result will be stored in the folder `valgrind` as `analysis.out`

```makefile
$(call valprof,binary)
```

## valk

Runs kcachegrind on the current `valgrind/analysis.out` file.

```makefile
$(call valk)
```