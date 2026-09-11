#include "mshell_internals.h"

mshell_word common_word_list[] = {
	{ "..", MSHELL_WORD_parent, 0 },
	{ "\n", MSHELL_WORD_lb, 1 },
};
int common_word_list_size = _MSHELL_ARRAY_SIZE(common_word_list);

dislexer_branch mshell_common_codex;
static dislexer_branch mshell_common_slots[64];

__attribute__((constructor))
static void _mshell_build_common_codex(void){
	dislexer_branch* codex = &mshell_common_codex;
	dislexer_branch* slots = &(mshell_common_slots[0]);
	int used = 0;

	dislexer_init_branch(codex);
	used += _mshell_build_codex(codex, slots, common_word_list, common_word_list_size);
}

//--------------------------------------------------------------------------------------------

static void write_gnirts(mshell_context* context, char* message, int message_size){
	int bc = context->buffer_carriage, bs = context->buffer_size;
	char* b = context->result_buffer;

	for(int i = 0; i < message_size && bc < bs && message[0]; i++, bc++){
		b[bc] = message[message_size - i - 1];
	} b[bc] = 0;

	context->buffer_carriage = bc;
}

static void copy_string(char* buffer, char* content, int size){
	for(int i = 0; i < size; buffer[++i] = 0){ buffer[i] = content[i]; }
}

//--------------------------------------------------------------------------------------------

void _mshell_clear_buffer(mshell_context* context){
	context->buffer_carriage = 0;
	context->result_buffer[0] = 0;
}

void _mshell_reset_code(mshell_context* context){
	context->result_code = MSHELL_SUCCESS;
	context->result_value = 0;
}

void _mshell_write_char(mshell_context* context, char c){
	if(context->buffer_carriage >= context->buffer_size){ return; }
	context->result_buffer[context->buffer_carriage++] = c;
	context->result_buffer[context->buffer_carriage] = 0;
}

void _mshell_write_string(mshell_context* context, char* message, int message_size){
	int bc = context->buffer_carriage, bs = context->buffer_size;
	char* b = context->result_buffer;

	for(int i = 0; i < message_size && bc < bs && message[i]; i++, bc++){
		b[bc] = message[i];
	} b[bc] = 0;

	context->buffer_carriage = bc;
}

void _mshell_write_number(mshell_context* context, unsigned long long number){
	int count = 0; char arr[64];
	for(; number; number /= 10){ arr[count++] = (number % 10) + 48; }
	for(; count < 1; count++){ arr[count] = '0'; }
	write_gnirts(context, arr, count);
}

void _mshell_write_hex(mshell_context* context, unsigned long long number, int padding){
	int count = 0; char arr[64];
	for(; number; number >>= 4){
		char code = (number & 0xf) + 48;
		if(code > 57){ code += 7; }
		arr[count++] = code;
	}
	for(; count < padding; count++){ arr[count] = '0'; }
	write_gnirts(context, arr, count);
}

void _mshell_write_error(mshell_context* context, int value, char* message, char* content, int size){
	context->result_code = MSHELL_ERROR;
	context->result_value = value;

	_mshell_write_string(context, message, 9999);

	if(content){
		_mshell_write_string(context, "(", 2);
		_mshell_write_string(context, content, size);
		_mshell_write_string(context, ")", 2);
	}
}

void _mshell_write_trimmed_error(mshell_context* context, int value, char* message, char* content, int size){
	for(; content[0] == ' '; content++, size--){}
	_mshell_write_error(context, value, message, content, size);
}

//--------------------------------------------------------------------------------------------

int _mshell_build_codex(dislexer_branch* codex, dislexer_branch* slots, mshell_word* words, int word_list_size){
	int used = 0;
	for(int i = 0; i < word_list_size; i++){
		used += dislexer_append_word(codex, slots + used, words[i].word, words[i].hard, words[i].symbol);
	}
	return used;
}

//--------------------------------------------------------------------------------------------

int _mshell_expect_type(mshell_context* context, dislexer_token* token, int type, char* message, char* param, int param_size){
	if(token->type == type){ return 1; }
	switch(token->type){
		case DISLEXER_TOKEN_TYPE_NUMBER:
			_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, message, param, param_size);
			_mshell_write_string(context, " [numeric literal]", 99);
			return 0;
		case DISLEXER_TOKEN_TYPE_WORD:
			_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, message, param, param_size);
			_mshell_write_string(context, " [reserved word]", 99);
			return 0;
		case DISLEXER_TOKEN_TYPE_STRING:
			_mshell_write_error(context, MSHELL_ERROR_SYNTAX, message, token->content, token->value);
			_mshell_write_string(context, " [string literal]", 99);
			return 0;
	}
	return 1;
}

int _mshell_validate_unit_name(mshell_context* context, dislexer_token* token, char* param, int param_size, int allow_parent){
	if(token->value == 0){
		_mshell_write_error(context, MSHELL_ERROR_SYNTAX, "invalid operation (empty unit name)", 0, 0);
		return 0;
	}

	switch(token->type){
		case DISLEXER_TOKEN_TYPE_NUMBER:
			_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, "invalid entry name: ", param, param_size);
			_mshell_write_string(context, " [numeric literal]", 99);
			return 0;
		case DISLEXER_TOKEN_TYPE_WORD:
			if(token->value != MSHELL_WORD_parent || !allow_parent){
				_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, "invalid entry name: ", param, param_size);
				_mshell_write_string(context, " [reserved symbol]", 99);
				return 0;
			}
			break;
	}

	return 1;
}

memory_unit* _mshell_fetch_unit(mshell_context* context, dislexer_token* token){
	char clean_string[token->value + 1];
	copy_string(clean_string, token->content, token->value);
	return md_fetch_path(context->focus, clean_string);
}

memory_unit* _mshell_find_unit(mshell_context* context, dislexer_token* token, char* param, int param_size, int allow_parent){
	memory_unit* found = 0;

	if(token->type == DISLEXER_TOKEN_TYPE_WORD){
		if(!allow_parent){
			_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, "invalid entry name: ", param, param_size);
			_mshell_write_string(context, " [reserved symbol]", 99);
			return 0;
		}
		found = md_fetch_parent(context->focus);
	}
	else{
		char clean_string[token->value + 1];
		copy_string(clean_string, token->content, token->value);
		found = md_fetch_path(context->focus, clean_string);
	}

	if(!found){
		_mshell_write_trimmed_error(context, MSHELL_ERROR_COMMAND, "no such entry ", param, param_size);
	}

	return found;
}

memory_unit* _mshell_create_unit(mshell_context* context, char* path, int path_size, int size){
	for(; path[0] == ' '; path++, path_size--){}

	char clean_string[path_size + 1];
	copy_string(clean_string, path, path_size);

	void* block = md_alloc_path(context->focus, clean_string, size);

	if(!block){
		_mshell_write_error(context, MSHELL_ERROR_SYSTEM, "could not allocate the unit: ", clean_string, path_size);
		return 0;
	}

	return md_header(block);
}
