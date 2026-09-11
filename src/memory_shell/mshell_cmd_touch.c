#include "mshell_internals.h"

int _mshell_command_touch(mshell_context* context, char* param){
	dislexer_token token; int param_size = dislexer_parse(&mshell_common_codex, param, &token);
	_mshell_clear_buffer(context);

	if(!_mshell_expect_type(context, &token, DISLEXER_TOKEN_TYPE_STRING, "invalid entry name ", param, param_size)){
		return 0;
	}

	dislexer_token token_2; int param_size_2 = dislexer_parse(&mshell_common_codex, param + param_size, &token_2);

	int size = 0;

	if(token_2.value){
		if(!_mshell_expect_type(context, &token_2, DISLEXER_TOKEN_TYPE_NUMBER, "invalid file size format ", param + param_size, param_size_2)){
			return 0;
		}
		size = token_2.value;
	}

	memory_unit* entry = _mshell_fetch_unit(context, &token);

	if(entry){
		_mshell_write_error(context, MSHELL_ERROR_COMMAND, "an entry with this name already exists ", 0, 0);
		return 0;
	}

	entry = _mshell_create_unit(context, token.content, token.value, size);

	if(!entry){ return 0; }

	return 1;
}
