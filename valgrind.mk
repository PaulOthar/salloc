#This is a linux only script

__valgrind_mk_DIR = ./valgrind
__valgrind_mk_OUT = $(__valgrind_mk_DIR)/analysis.out

__valgrind_mk_FLAGS = --tool=callgrind
__valgrind_mk_FLAGS += --dump-line=yes
__valgrind_mk_FLAGS += --dump-instr=yes
__valgrind_mk_FLAGS += --collect-jumps=yes
__valgrind_mk_FLAGS += --collect-systime=yes
__valgrind_mk_FLAGS += --collect-bus=yes
__valgrind_mk_FLAGS += --cache-sim=yes
__valgrind_mk_FLAGS += --branch-sim=yes
__valgrind_mk_FLAGS += --simulate-wb=yes
__valgrind_mk_FLAGS += --simulate-hwpref=yes
__valgrind_mk_FLAGS += --cacheuse=yes
__valgrind_mk_FLAGS += --time-stamp=yes

define valprof
	@if [ ! -d "$(__valgrind_mk_DIR)" ]; then echo "Creating valgrind directory"; mkdir $(__valgrind_mk_DIR); fi
	@if [ -f "$(__valgrind_mk_OUT)" ]; then echo "Removing previous results"; rm -f $(__valgrind_mk_OUT); fi
	
	@valgrind $(__valgrind_mk_FLAGS) --callgrind-out-file=$(__valgrind_mk_OUT) $(1)
endef

define valk
	@if [ ! -f "$(__valgrind_mk_OUT)" ]; then echo "There is no result to open in kcachegrind"; exit 0; fi
	
	@sudo kcachegrind $(__valgrind_mk_OUT)
endef