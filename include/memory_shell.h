#ifndef MEMORY_INSPECTOR
#define MEMORY_INSPECTOR

#include "memory_directory.h"

typedef enum {
    MSHELL_SUCCESS	= 0,
	MSHELL_ERROR	= 1,
	MSHELL_BUSY		= 2,
	MSHELL_CUSTOM	= 3,
} memory_shell_code;

typedef enum {
    MSHELL_ERROR_LEXICAL	= 1,
	MSHELL_ERROR_SYNTAX		= 2,
	MSHELL_ERROR_COMMAND	= 3,
	MSHELL_ERROR_SYSTEM		= 4,
} memory_shell_error;

typedef struct /*memory_context*/{
	memory_unit* root;
	memory_unit* focus;

	int result_code;
	int result_value;

	char* result_buffer;
	unsigned int buffer_size;
	unsigned int buffer_carriage;

	int default_result_lines;
}mshell_context;

void mshell_context_init(mshell_context* context, memory_unit* root, char* result_buffer, int buffer_size);

int mshell_run(mshell_context* context, char* command);

#endif
