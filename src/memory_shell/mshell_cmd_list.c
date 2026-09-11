#include "mshell_internals.h"

typedef enum {
	MSHELL_WORD_lines = 1,
	MSHELL_WORD_offset,
} mshell_local_symbol;

static mshell_word local_word_list[] = {
	{ "-l", MSHELL_WORD_lines, 0 },
	{ "-o", MSHELL_WORD_offset, 0 },
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

int _mshell_command_list(mshell_context* context, char* param){
	dislexer_token token; int param_size = dislexer_parse(&mshell_local_codex, param, &token);
	_mshell_clear_buffer(context);

	int lines = context->default_result_lines;
	int offset = 0;

	for(int word = token.value; word; word = token.value){
		if(!_mshell_expect_type(context, &token, DISLEXER_TOKEN_TYPE_WORD, "invalid parameter ", param, param_size)){
			return 0;
		}

		if(word == MSHELL_WORD_parent){
			_mshell_write_trimmed_error(context, MSHELL_ERROR_LEXICAL, "invalid parameter ", param, param_size);
			return 0;
		}

		param += param_size;
		param_size = dislexer_parse(&mshell_local_codex, param, &token);//Step to this parameter's value
		if(!_mshell_expect_type(context, &token, DISLEXER_TOKEN_TYPE_NUMBER, "invalid value ", param, param_size)){
			return 0;
		}

		switch(word){
			case MSHELL_WORD_lines: lines = token.value; break;
			case MSHELL_WORD_offset: offset = token.value; break;
			default:
				_mshell_write_trimmed_error(context, MSHELL_ERROR_LEXICAL, "invalid parameter ", param, param_size);
				return 0;
		}

		//Step to the next token
		param += param_size;
		param_size = dislexer_parse(&mshell_local_codex, param, &token);
	}

	int found = 0;
	int skipped = 0;
	for(memory_unit* inner = context->focus->children; inner && found < lines; inner = inner->next){
		if(skipped < offset){ skipped++; continue; }

		if(found){ _mshell_write_string(context, "\n", 2); }
		_mshell_write_string(context, inner->name, 16);
		_mshell_write_string(context, " - Total: ", 99);
		_mshell_write_number(context, md_recursive_size(inner));
		if(inner->size){
			_mshell_write_string(context, " (Self: ", 99);
			_mshell_write_number(context, inner->size);
			_mshell_write_string(context, ")", 3);
		}
		found++;
	}

	if(!found){
		if(!skipped && lines){ _mshell_write_string(context, "this directory is empty", 99); }
		else{ _mshell_write_string(context, "no entry found in the specified configuration", 99); }
	}

	return 1;
}

