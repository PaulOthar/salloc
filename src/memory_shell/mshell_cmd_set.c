#include "mshell_internals.h"

typedef enum {
	MSHELL_WORD_offset = 1,
	MSHELL_WORD_repeat = 2,
} mshell_local_symbol;

static mshell_word local_word_list[] = {
	{ "-o", MSHELL_WORD_offset, 0 },
	{ "-r", MSHELL_WORD_repeat, 0 },
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

typedef struct _token_list{
	dislexer_token* token;
	struct _token_list* next;
}token_list;

static int write_token_data_recursive(mshell_context* context, int offset, int repeat, char* param, token_list* start, token_list* end);
static int write_tokens_to_unit(memory_unit* target, int offset, int repeat, token_list* start);
static int append_string(char* block, int limit, int offset, char* str, int str_size);
static int append_number(char* block, int limit, int offset, unsigned long long int number);

int _mshell_command_set(mshell_context* context, char* param){
	dislexer_token token; int param_size = dislexer_parse(&mshell_local_codex, param, &token);
	_mshell_clear_buffer(context);

	int offset = 0;
	int repeat = 1;
	memory_unit* target = context->focus;

	if(token.type != DISLEXER_TOKEN_TYPE_WORD){
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
		if(token.type != DISLEXER_TOKEN_TYPE_WORD){ break; }

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
			case MSHELL_WORD_offset: offset = token.value; break;
			case MSHELL_WORD_repeat: repeat = token.value; break;
			default:
				_mshell_write_trimmed_error(context, MSHELL_ERROR_LEXICAL, "invalid parameter ", param, param_size);
				return 0;
		}

		//Step to the next token
		param += param_size;
		param_size = dislexer_parse(&mshell_local_codex, param, &token);
	}

	memory_unit* prev = context->focus;
	context->focus = target;
	int result = write_token_data_recursive(context, offset, repeat, param, 0, 0);
	context->focus = prev;

	if(context->flags & MSHELL_FLAG_HEX_WHEN_SET){
		return _mshell_hex(context, target, context->default_result_lines, offset, 8);
	}

	return result;
}

static int write_token_data_recursive(mshell_context* context, int offset, int repeat, char* param, token_list* start, token_list* end){
	dislexer_token token; int param_size = dislexer_parse(&mshell_local_codex, param, &token);

	if(!token.value && token.type != DISLEXER_TOKEN_TYPE_NUMBER){//we reached the end of this ordeal
		write_tokens_to_unit(context->focus, offset, repeat, start);
		return 1;
	}

	if(token.type == DISLEXER_TOKEN_TYPE_WORD){
		_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, "invalid input ", param, param_size);
		_mshell_write_string(context, " [reserved word]", 99);
		return 0;
	}

	//now token MUST be a number or a string

	token_list this = { &token, 0 };
	if(!start){ start = &this; }
	if(end){ end->next = &this; }
	end = &this;
	return write_token_data_recursive(context, offset, repeat, param + param_size, start, end);
}

static int write_tokens_to_unit(memory_unit* target, int offset, int repeat, token_list* start){
	int size = target->size;
	char* block = md_data(target);

	for(int i = 0; i < repeat; i++){
		for(token_list* list = start; list && offset < size; list = list->next){
			dislexer_token* data = list->token;

			if(data->type == DISLEXER_TOKEN_TYPE_NUMBER){
				offset = append_number(block, size, offset, data->value);
				continue;
			}
			offset = append_string(block, size, offset, data->content, data->value);
		}
	}

	return 1;
}

static int append_number(char* block, int limit, int offset, unsigned long long int number){
	if(!number && offset < limit){ block[offset++] = number; }
	for(; number && offset < limit; number >>= 8, offset++){
		block[offset] = number & 0xff;
	}
	return offset;
}

static int append_string(char* block, int limit, int offset, char* str, int str_size){
	for(int i = 0; i < str_size && str[i] && offset < limit; i++, offset++){
		block[offset] = str[i];
	}
	return offset;
}
