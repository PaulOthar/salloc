#include "mshell_internals.h"

typedef enum {
	MSHELL_WORD_lines = 1,
	MSHELL_WORD_offset,
	MSHELL_WORD_bytes,
} mshell_local_symbol;

static mshell_word local_word_list[] = {
	{ "-l", MSHELL_WORD_lines, 0 },
	{ "-o", MSHELL_WORD_offset, 0 },
	{ "-b", MSHELL_WORD_bytes, 0 },
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

int _mshell_hex(mshell_context* context, memory_unit* target, int lines, int offset, int bytes){
	char* block = md_data(target);
	if(!block){
		_mshell_write_error(context, MSHELL_ERROR_COMMAND, "entry has no memory ", 0, 0);
		return 0;
	}

	int size = target->size;
	int reading = (lines * bytes) + offset;
	size = size < reading ? size : reading;

	for(int i = offset; i < size; i += bytes){
		int ipr = i + bytes;
		if(ipr >= size){ ipr = size; }

		if(i){ _mshell_write_string(context, "\n", 2); }
		_mshell_write_hex(context, i, 8); _mshell_write_string(context, " | ", 4);

		for(int l = i; l < ipr; l++){
			if(l > i){ _mshell_write_string(context, " ", 2); }
			_mshell_write_hex(context, block[l] & 0xff, 2);
		}

		for(int l = i + bytes - size; l > 0; l--){
			_mshell_write_string(context, " __", 4);
		}

		_mshell_write_string(context, " | ", 4);

		for(int l = i; l < ipr; l++){
			int c = block[l];
			if(c < 32 || c > 127){ c = '.'; }
			_mshell_write_char(context, c);
		}
	}

	return 1;
}

int _mshell_command_hex(mshell_context* context, char* param){
	dislexer_token token; int param_size = dislexer_parse(&mshell_local_codex, param, &token);
	_mshell_clear_buffer(context);

	int lines = context->default_result_lines;
	int offset = 0;
	int bytes = 8;
	memory_unit* target = context->focus;

	if(token.type == 0 && token.value == 0){  }
	else if(token.type != DISLEXER_TOKEN_TYPE_WORD){
		target = _mshell_find_unit(context, &token, param, param_size, 0);
		if(!target){ return 0; }

		param += param_size;
		param_size = dislexer_parse(&mshell_local_codex, param, &token);
	}

	char* block = md_data(target);
	if(!block){
		_mshell_write_error(context, MSHELL_ERROR_COMMAND, "entry has no memory ", 0, 0);
		return 0;
	}

	for(int word = token.value; word; word = token.value){
		if(!_mshell_expect_type(context, &token, DISLEXER_TOKEN_TYPE_WORD, "invalid parameter ", param, param_size)){
			return 0;
		}//it is a word now

		if(word == MSHELL_WORD_parent){
			_mshell_write_trimmed_error(context, MSHELL_ERROR_LEXICAL, "invalid parameter ", param, param_size);
			return 0;
		}

		param += param_size;
		param_size = dislexer_parse(&mshell_local_codex, param, &token);//Step to this parameter's value
		if(!_mshell_expect_type(context, &token, DISLEXER_TOKEN_TYPE_NUMBER, "invalid parameter value ", param, param_size)){
			return 0;
		}

		switch(word){
			case MSHELL_WORD_lines: lines = token.value; break;
			case MSHELL_WORD_offset: offset = token.value; break;
			case MSHELL_WORD_bytes: bytes = token.value; break;
			default:
				_mshell_write_trimmed_error(context, MSHELL_ERROR_LEXICAL, "invalid parameter ", param, param_size);
				return 0;
		}

		//Step to the next token
		param += param_size;
		param_size = dislexer_parse(&mshell_local_codex, param, &token);
	}

	return _mshell_hex(context, target, lines, offset, bytes);
}
