#ifndef MEMORY_SHELL_INTERNALS
#define MEMORY_SHELL_INTERNALS

#include "memory_shell.h"
#include "dislexer.h"
#include <stdio.h>

#define _MSHELL_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

//----------------internal construction----------------

typedef struct {
	char* word;
	int symbol;
	int hard;
}mshell_word;

int _mshell_build_codex(dislexer_branch* codex, dislexer_branch* slots, mshell_word* words, int word_list_size);

typedef enum {
	MSHELL_WORD_parent = 0x800000,
	MSHELL_WORD_lb = 0,
} mshell_common_symbol;

extern mshell_word common_word_list[];
extern int common_word_list_size;

extern dislexer_branch mshell_common_codex;

//----------------internal utility----------------

void _mshell_clear_buffer(mshell_context* context);
void _mshell_reset_code(mshell_context* context);

void _mshell_write_char(mshell_context* context, char c);
void _mshell_write_string(mshell_context* context, char* message, int message_size);
void _mshell_write_number(mshell_context* context, unsigned long long number);
void _mshell_write_hex(mshell_context* context, unsigned long long number, int padding);
void _mshell_write_error(mshell_context* context, int value, char* message, char* content, int size);
void _mshell_write_trimmed_error(mshell_context* context, int value, char* message, char* content, int size);

int _mshell_expect_type(mshell_context* context, dislexer_token* token, int type, char* message, char* param, int param_size);

int _mshell_validate_unit_name(mshell_context* context, dislexer_token* token, char* param, int param_size, int allow_parent);
memory_unit* _mshell_fetch_unit(mshell_context* context, dislexer_token* token);
memory_unit* _mshell_find_unit(mshell_context* context, dislexer_token* token, char* param, int param_size, int allow_parent);
memory_unit* _mshell_create_unit(mshell_context* context, char* path, int path_size, int size);

//----------------reusable commands----------------

int _mshell_list(mshell_context* context, int lines, int offset);
int _mshell_hex(mshell_context* context, memory_unit* target, int lines, int offset, int bytes);

//----------------internal commands----------------

int _mshell_command_change(mshell_context* context, char* param);
int _mshell_command_list(mshell_context* context, char* param);
int _mshell_command_touch(mshell_context* context, char* param);
int _mshell_command_remove(mshell_context* context, char* param);

int _mshell_command_stat(mshell_context* context, char* param);
int _mshell_command_hex(mshell_context* context, char* param);
int _mshell_command_set(mshell_context* context, char* param);

int _mshell_command_mem(mshell_context* context, char* param);

#endif
