# util.mk

This is a collection of utility functions that some other scripts may use. 
Functions in this file are designed to be as simple as possible, and provide support to Windows and Linux.

## mkdir

Creates a directory with the specified name/path. 
You must avoid using quotation marks, since it can lead to some errors.

```makefile
$(call mkdir,name)
```

## rm_fil

Removes/deletes the specified file. 
You must avoid using quotation marks, since it can lead to some errors.

```makefile
$(call rm_fil,file_name)
```

## rm_dir

Removes/deletes the specified directory recursively. 
You must avoid using quotation marks, since it can lead to some errors.

```makefile
$(call rm_dir,directory_name)
```

## if_fil

Checks if the specified file exists, and if it does, executes the specified command.
You must avoid using quotation marks, since it can lead to some errors.

```makefile
$(call if_fil,file_name,command)
```

## run_make

Attempts to run a make target on the specified directory. If successifull it shows nothing, if it failed, will show an error message.

```makefile
$(call run_make,directory,target)
```

## run_make_in_subdirs

Attempts to run a make target in every subdirectory of the specified directory. If successifull it shows nothing, if it failed, will show an error message.

```makefile
$(call run_make_in_subdirs,directory,target)
```

## rwildcard

Recursively fetches every file from every subdirectory in the specified direction, with the specified pattern.

```makefile
$(call rwildcard,directory,pattern)
```