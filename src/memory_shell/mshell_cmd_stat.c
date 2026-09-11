#include "mshell_internals.h"

typedef enum {
	MSHELL_WORD_id 				= 0b00000001,
	MSHELL_WORD_name 			= 0b00000010,
	MSHELL_WORD_father			= 0b00000100,
	MSHELL_WORD_size			= 0b00001000,

	MSHELL_WORD_total_size		= 0b00010000,
	MSHELL_WORD_branches 		= 0b00100000,

	MSHELL_WORD_data_address	= 0b01000000,
} mshell_local_symbol;

static mshell_word local_word_list[] = {
	{ "-id", MSHELL_WORD_id, 0 },
	{ "-n", MSHELL_WORD_name, 0 },
	{ "-p", MSHELL_WORD_father, 0 },
	{ "-s", MSHELL_WORD_size, 0 },
	{ "-t", MSHELL_WORD_total_size, 0 },
	{ "-b", MSHELL_WORD_branches, 0 },
	{ "-ptr", MSHELL_WORD_data_address, 0 },
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

int _mshell_command_stat(mshell_context* context, char* param){
	dislexer_token token; int param_size = dislexer_parse(&mshell_local_codex, param, &token);
	_mshell_clear_buffer(context);

	int mode = 0;
	memory_unit* target = context->focus;

	if(token.type == DISLEXER_TOKEN_TYPE_NUMBER){
		_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, "invalid entry name ", param, param_size);
		return 0;
	}

	if(token.type == DISLEXER_TOKEN_TYPE_STRING){
		target = _mshell_find_unit(context, &token, param, param_size, 0);
		if(!target){ return 0; }

		param += param_size;
		param_size = dislexer_parse(&mshell_local_codex, param, &token);
	}

	for(int word = token.value; word; word = token.value){
		if(!_mshell_expect_type(context, &token, DISLEXER_TOKEN_TYPE_WORD, "invalid parameter ", param, param_size)){
			return 0;
		}//it is a word now

		switch(token.value){
			case MSHELL_WORD_id:
			case MSHELL_WORD_name:
			case MSHELL_WORD_father:
			case MSHELL_WORD_size:
			case MSHELL_WORD_total_size:
			case MSHELL_WORD_branches:
			case MSHELL_WORD_data_address:
				mode |= token.value;
				break;
			default:
				_mshell_write_trimmed_error(context, MSHELL_ERROR_SYNTAX, "invalid entry ", param, param_size);
				return 0;
				break;
		}

		param += param_size;
		param_size = dislexer_parse(&mshell_local_codex, param, &token);
	}

	if(!mode){ mode = 0xff; }

	if(mode & MSHELL_WORD_id){
		_mshell_write_string(context, "id: ", 99);
		_mshell_write_hex(context, target->id, 16);
		_mshell_write_string(context, "\n", 99);
	}

	if(mode & MSHELL_WORD_name){
		_mshell_write_string(context, "name: ", 99);
		_mshell_write_string(context, target->name, 16);
		_mshell_write_string(context, "\n", 99);
	}

	if(mode & MSHELL_WORD_father){
		_mshell_write_string(context, "parent: ", 99);
		memory_unit* parent = md_fetch_parent(target);
		_mshell_write_string(context, parent ? parent->name : "---", 16);
		_mshell_write_string(context, "\n", 99);
	}

	if(mode & MSHELL_WORD_size){
		_mshell_write_string(context, "size: ", 99);
		_mshell_write_number(context, target->size);
		_mshell_write_string(context, "\n", 99);
	}

	if(mode & MSHELL_WORD_total_size){
		_mshell_write_string(context, "total size: ", 99);
		_mshell_write_number(context, md_recursive_size(target));
		_mshell_write_string(context, "\n", 99);
	}

	if(mode & MSHELL_WORD_branches){
		_mshell_write_string(context, "total branches: ", 99);
		_mshell_write_number(context, md_recursive_count(target));
		_mshell_write_string(context, "\n", 99);
	}

	if(mode & MSHELL_WORD_data_address){
		_mshell_write_string(context, "data ptr: ", 99);
		_mshell_write_hex(context, (uintptr_t)target->data, 16);
		_mshell_write_string(context, "\n", 99);
	}

	return 0;
}
