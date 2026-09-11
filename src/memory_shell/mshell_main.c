#include "mshell_internals.h"

typedef int (*command_handler)(mshell_context* context, char* param);

typedef struct /*shell_command*/{
	char* word;
	command_handler command;
}shell_command;

static shell_command call_table[] = {
	{ "cd", _mshell_command_change },
	{ "ls", _mshell_command_list },
	{ "touch", _mshell_command_touch },
	{ "rm", _mshell_command_remove },

	{ "stat", _mshell_command_stat },
	{ "hex", _mshell_command_hex },
	{ "set", _mshell_command_set },
//
//	{ "mem", _mshell_command_mem },
};
int shell_command_table_size = _MSHELL_ARRAY_SIZE(call_table);

typedef enum {
	MSHELL_WORD_exit = _MSHELL_ARRAY_SIZE(call_table),
} mshell_symbol;

static mshell_word word_list[] = {
	{ "exit", MSHELL_WORD_exit, 0 },
};
int word_list_size = _MSHELL_ARRAY_SIZE(word_list);

static dislexer_branch mshell_local_codex;
static dislexer_branch mshell_local_slots[64];

__attribute__((constructor))
static void _mshell_build_local_codex(void){
	dislexer_branch* codex = &mshell_local_codex;
	dislexer_branch* slots = &(mshell_local_slots[0]);
	int used = 0;

	dislexer_init_branch(codex);
	for(int i = 0; i < shell_command_table_size; i++){
		used += dislexer_append_word(codex, slots + used, call_table[i].word, 0, i);
	}

	used += _mshell_build_codex(codex, slots + used, word_list, word_list_size);
}

void mshell_context_init(mshell_context* context, memory_unit* root, char* result_buffer, int buffer_size){
	context->root = root;
	context->result_buffer = result_buffer;
	context->buffer_size = buffer_size;

	context->focus = root;

	context->result_code = 0;
	context->result_value = 0;

	context->default_result_lines = 16;
}

//----------------run implementation----------------

int mshell_run(mshell_context* context, char* command){
	dislexer_token token; int command_size = dislexer_parse(&mshell_local_codex, command, &token);
	_mshell_clear_buffer(context);

	if(!_mshell_expect_type(context, &token, DISLEXER_TOKEN_TYPE_WORD, "no such command ", command, command_size)){
		return 0;
	}

	int result = 0;
	switch(token.value){
		default:
			if(token.value > shell_command_table_size){ return 0; }
			_mshell_reset_code(context);
			result = call_table[token.value].command(context, command + command_size);
			break;
		case MSHELL_WORD_exit:
			context->result_code = MSHELL_SUCCESS;
			context->result_value = -1;
			result = -1; break;
	}

	return result;
}
