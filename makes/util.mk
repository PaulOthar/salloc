ifeq ($(OS),Windows_NT)
	OSTYPE = Windows
	
define winpath
$(subst /,\\,$(1))
endef
	
define mkdir
if not exist "$(call winpath,$(1))" mkdir "$(call winpath,$(1))"
endef
	
define rm_fil
if exist "$(call winpath,$(1))" del /Q "$(call winpath,$(1))"
endef
	
define rm_dir
if exist "$(call winpath,$(1))" rmdir /S /Q "$(call winpath,$(1))"
endef

define if_fil
if exist "$(call winpath,$(1))" $(2)
endef

define run_make
	$(MAKE) --no-print-directory -C "$(1)" $(2) 2>NUL || echo "ERROR: $(1) -> $(2) failed"
endef

else
	UNAME = $(shell uname -s)

	ifeq ($(UNAME),Linux)
	OSTYPE = Linux
		
define mkdir
mkdir -p "$(1)"
endef
	
define rm_fil
rm -f "$(1)"
endef
	
define rm_dir
rm -rf "$(1)"
endef

define if_fil
if [ -f "$(1)" ]; then $(2); fi
endef

define run_make
	$(MAKE) --no-print-directory -C "$(1)" $(2) 2>/dev/null || echo "ERROR: $(1) -> $(2) failed"
endef
	
	endif
endif

#Silly goofy busyness so commands can coexist with echoes witheout concatenation
define newline


endef

#run_make_in_subdirs(directory, target)
define run_make_in_subdirs
$(foreach dir,$(wildcard $(1)/*/),$(call run_make,$(dir),$(2))$(newline))
endef

#recursive_wildcard(directory, pattern)
#1.1| gets all files in this directory with the specified pattern
#2.1| for each subdirectory in this directory
#2.2| run this nonsense again in the subdirectory we found
define rwildcard
$(wildcard $(1)/$(2))$(foreach dir,$(wildcard $(1)/*/),$(call rwildcard,$(dir),$(2)))
endef