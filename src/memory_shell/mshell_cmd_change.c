#include "mshell_internals.h"

int _mshell_command_change(mshell_context* context, char* param){
	dislexer_token token; int param_size = dislexer_parse(&mshell_common_codex, param, &token);
	_mshell_clear_buffer(context);

	if(!_mshell_validate_unit_name(context, &token, param, param_size, 1)){ return 0; }

	memory_unit* entry = _mshell_find_unit(context, &token, param, param_size, 1);
	if(!entry){ return 0; }

	context->focus = entry;

	if(context->flags & MSHELL_FLAG_LIST_WHEN_CHANGE){
		return _mshell_list(context, context->default_result_lines, 0);
	}

	return 1;
}
