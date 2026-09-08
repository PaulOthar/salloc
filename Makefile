include makes/util.mk

SRC_DIR = ./src
INC_DIR = ./include
BIN_DIR = ./bin

SRC = $(call rwildcard,$(SRC_DIR),*.c)
SRC += $(call rwildcard,./test,*.c)
OUTPUT = $(BIN_DIR)/bin

CFLAGS = -Wall -Wextra -O0 -g3

CC = gcc

#CFLAGS += -DSTATIC_ALLOCATOR_DEBUG_MODE
#CFLAGS += -DMEMORY_DIRECTORY_DEBUG_MODE

LIBS = -I$(LIB_DIR)/dislexer/include -L$(LIB_DIR)/dislexer/lib/$(OSTYPE) -ldislexer

include makes/release.mk
all:
	@$(call build_dependency,./lib)
	$(CC) $(SRC) -o $(OUTPUT) -I$(INC_DIR) $(CFLAGS)

include makes/valgrind.mk
mcall:
	$(call valprof,$(OUTPUT))
	$(call valk)

WASM_CC = emcc
WASM_DIR = ./wasm
WASM_OUT = $(WASM_DIR)/code.js
WASM_FLG = -sEXIT_RUNTIME=1 -sFORCE_FILESYSTEM=1

wall:
	$(WASM_CC) $(SRC) -o $(WASM_OUT) -I$(INC_DIR) $(CFLAGS) $(WASM_FLG)

LIBNAME = s_alloc

release-fix:
	@echo "$(LIBNAME) -> Fixing makes submodules"
	@git submodule update --init makes
	@$(call run_make_in_subdirs,./lib,release-fix)

release:
	$(call build_release,$(LIBNAME))