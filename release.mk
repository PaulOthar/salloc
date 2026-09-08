include makes/util.mk

__release_mk_REL_DIR = ./release
__release_mk_REL_DIR_LIB = $(__release_mk_REL_DIR)/lib
__release_mk_REL_DIR_OBJ = $(__release_mk_REL_DIR)/obj

__release_mk_INC_DIR = ./include
__release_mk_SRC_DIR = ./src

__release_mk_CC = gcc
__release_mk_AR = ar
__release_mk_RANLIB = ranlib
__release_mk_CFLAGS = -O2 #-Wall -Wextra -g3

__release_mk_REL_SRC = $(call rwildcard,$(__release_mk_SRC_DIR),*.c)
#Makefile translation: 
#	$(patsubst pattern, replace, text)
#	for each pattern in text, apply the replacement
__release_mk_REL_OBJECTS = $(patsubst $(__release_mk_SRC_DIR)/%.c,$(__release_mk_REL_DIR_OBJ)/%.o,$(__release_mk_REL_SRC))

#Makefile translation: 
#	"$<" = dependency = 'src/code.c'
#	"$@" = target = 'tmp/code.o'
$(__release_mk_REL_DIR_OBJ)/%.o: $(__release_mk_SRC_DIR)/%.c
	@$(call mkdir,$(dir $@))
	@$(__release_mk_CC) -c $< -o $@ -I$(__release_mk_INC_DIR) $(__release_mk_CFLAGS)

#build_release(lib_name)
define build_release
	@$(call mkdir,$(__release_mk_REL_DIR))
	@$(call mkdir,$(__release_mk_REL_DIR_LIB))
	
	@$(call rm_dir,$(__release_mk_REL_DIR_OBJ))
	@echo "LIB - $(1) - Building object files"
	@$(MAKE) $(__release_mk_REL_OBJECTS)
	
	@echo "LIB - $(1) - Building archive file"
	@$(call rm_fil,$(__release_mk_REL_DIR_LIB)/lib$(1).a)
	@$(__release_mk_AR) rc "$(__release_mk_REL_DIR_LIB)/lib$(1).a" $(__release_mk_REL_OBJECTS)
	@$(__release_mk_RANLIB) "$(__release_mk_REL_DIR_LIB)/lib$(1).a"
	
	@echo "LIB - $(1) - Done"
endef

# build_dependency(directory)
# for each project inside the specified directory,
# run its Makefile with the release target (if it has the file and the target)
define build_dependency
$(call run_make_in_subdirs,$(1),release)
endef