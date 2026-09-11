#include "mshell_internals.h"

typedef enum {
	MSHELL_WORD_recursive = 1
} mshell_local_symbol;

static mshell_word local_word_list[] = {
	{ "-r", MSHELL_WORD_recursive, 0 },
};
static int local_word_list_size = _MSHELL_ARRAY_SIZE(local_word_list);

static dislexer_branch mshell_local_codex;
static dislexer_branch mshell_local_slots[64];

__attribute__((constructor))
static void _mshell_build_local_codex(void){
	dislexer_branch* codex = &mshell_local_codex;
	dislexer_branch* slots = &(mshell_local_slots[0]);
	int used = 0;

	dislexer_init_branch(codex);

	used += _mshell_build_codex(codex, slots + used, common_word_list, common_word_list_size);
	used += _mshell_build_codex(codex, slots + used, local_word_list, local_word_list_size);
}

int _mshell_command_remove(mshell_context* context, char* param){
	dislexer_token token; int param_size = dislexer_parse(&mshell_local_codex, param, &token);
	_mshell_clear_buffer(context);

	int is_recursive = 0;

	switch(token.type){
		case DISLEXER_TOKEN_TYPE_STRING: break;
		case DISLEXER_TOKEN_TYPE_WORD: switch(token.value){
			case MSHELL_WORD_recursive:
				param += param_size;
				param_size = dislexer_parse(&mshell_local_codex, param, &token);
				is_recursive = 1;
				break;
			default:
				_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, "invalid parameter ", param, param_size);
				return 0;
		} break;
		default:
			_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, "invalid parameter ", param, param_size);
			return 0;
	}

	if(!_mshell_expect_type(context, &token, DISLEXER_TOKEN_TYPE_STRING, "invalid entry name ", param, param_size)){
		return 0;
	}

	memory_unit* entry = _mshell_find_unit(context, &token, param, param_size, 0);
	if(!entry){ return 0; }

	if(entry->children && !is_recursive){
		_mshell_write_error(context, MSHELL_ERROR_SYNTAX, "entry is not empty, use '-r' for recursive removal ", 0, 0);
		return 0;
	}

	md_free_unit(entry);

	return 1;
}
