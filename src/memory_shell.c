//#include "memory_shell.h"
//#include "static_allocator.h"
//#include "dislexer.h"
//
//#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
//
//typedef int (*command_handler)(mshell_context* context, char* param);
//
//typedef struct /*shell_command*/{
//	char* word;
//	command_handler command;
//}shell_command;
//
//static const shell_command call_table[] = {
//	{ "cd", _mshell_command_cd },
//	{ "ls", _mshell_command_ls },
//	{ "mkdir", _mshell_command_mkdir },
//	{ "touch", _mshell_command_touch },
//	{ "rm", _mshell_command_rm },
//
//	{ "hex", _mshell_command_hex },
//	{ "stat", _mshell_command_stat },
//	{ "set", _mshell_command_set },
//
//	{ "mem", _mshell_command_mem },
//};
//int shell_command_table_size = ARRAY_SIZE(call_table);
//
//typedef enum {
//	MEMORY_SHELL_WORD_exit = ARRAY_SIZE(call_table),
//	MEMORY_SHELL_WORD_dotdot,
//	MEMORY_SHELL_WORD_merge,
//
//	MEMORY_SHELL_WORD_h,
//	MEMORY_SHELL_WORD_r,
//	MEMORY_SHELL_WORD_o,
//	MEMORY_SHELL_WORD_l,
//	MEMORY_SHELL_WORD_t,
//
//	MEMORY_SHELL_WORD_line_break = 0,
//} memory_shell_symbol;
//
//typedef struct {
//	char* word;
//	memory_shell_symbol symbol;
//	int hard;
//}shell_symbol;
//
//static const shell_symbol symbol_table[] = {
//	{ "exit", MEMORY_SHELL_WORD_exit, 0 },
//	{ "..", MEMORY_SHELL_WORD_dotdot, 1 },
//	{ "-h", MEMORY_SHELL_WORD_h, 0 },
//	{ "-r", MEMORY_SHELL_WORD_r, 0 },
//	{ "-o", MEMORY_SHELL_WORD_o, 0 },
//	{ "-l", MEMORY_SHELL_WORD_l, 0 },
//	{ "merge", MEMORY_SHELL_WORD_merge, 0 },
//	{ "\n", MEMORY_SHELL_WORD_line_break, 1 },
//};
//int word_list_size = ARRAY_SIZE(symbol_table);
//
//dislexer_branch mshell_codex;
//dislexer_branch mshell_slots[64];
//
//__attribute__((constructor))
//void mshell_build_codex(void){
//	dislexer_branch* codex = &mshell_codex;
//	dislexer_branch* slots = &(mshell_slots[0]);
//	int used = 0;
//
//	dislexer_init_branch(codex);
//	for(int i = 0; i < shell_command_table_size; i++){
//		used += dislexer_append_word(codex, slots + used, call_table[i].word, 0, i);
//	}
//	for(int i = 0; i < word_list_size; i++){
//		used += dislexer_append_word(codex, slots + used, symbol_table[i].word, symbol_table[i].hard, symbol_table[i].symbol);
//	}
//}
//
//void mshell_context_init(mshell_context* context, memory_unit* root, char* result_buffer, int buffer_size){
//	context->root = root;
//	context->result_buffer = result_buffer;
//	context->buffer_size = buffer_size;
//
//	context->focus = root;
//
//	context->result_code = 0;
//	context->result_value = 0;
//}
//
//static void write_char(mshell_context* context, char c){
//	if(context->buffer_carriage >= context->buffer_size){ return; }
//	context->result_buffer[context->buffer_carriage++] = c;
//	context->result_buffer[context->buffer_carriage] = 0;
//}
//
//static void write_string(mshell_context* context, char* message, int message_size){
//	int bc = context->buffer_carriage, bs = context->buffer_size;
//	char* b = context->result_buffer;
//
//	for(int i = 0; i < message_size && bc < bs && message[i]; i++, bc++){
//		b[bc] = message[i];
//	} b[bc] = 0;
//
//	context->buffer_carriage = bc;
//}
//
//static void write_gnirts(mshell_context* context, char* message, int message_size){
//	int bc = context->buffer_carriage, bs = context->buffer_size;
//	char* b = context->result_buffer;
//
//	for(int i = 0; i < message_size && bc < bs && message[0]; i++, bc++){
//		b[bc] = message[message_size - i - 1];
//	} b[bc] = 0;
//
//	context->buffer_carriage = bc;
//}
//
//static void write_number(mshell_context* context, unsigned long long number){
//	int count = 0; char arr[64];
//	for(; number; number /= 10){ arr[count++] = (number % 10) + 48; }
//	for(; count < 1; count++){ arr[count] = '0'; }
//	write_gnirts(context, arr, count);
//}
//
//static void write_hex(mshell_context* context, unsigned long long number, int padding){
//	int count = 0; char arr[64];
//	for(; number; number >>= 4){
//		char code = (number & 0xf) + 48;
//		if(code > 57){ code += 7; }
//		arr[count++] = code;
//	}
//	for(; count < padding; count++){ arr[count] = '0'; }
//	write_gnirts(context, arr, count);
//}
//
//static void write_error(mshell_context* context, int value, char* message, char* content, int size){
//	context->result_code = MSHELL_ERROR;
//	context->result_value = value;
//
//	write_string(context, message, 9999);
//
//	if(content){
//		write_string(context, "(", 2);
//		write_string(context, content, size);
//		write_string(context, ")", 2);
//	}
//}
//
//static void clear_buffer(mshell_context* context){
//	context->buffer_carriage = 0;
//	context->result_buffer[0] = 0;
//}
//
////----------------run implementation----------------
//
//int mshell_run(mshell_context* context, char* command){
//	dislexer_token token; command += dislexer_parse(&mshell_codex, command, &token);
//	clear_buffer(context);
//
//	if(token.type == DISLEXER_TOKEN_TYPE_STRING){
//		write_error(context, MSHELL_ERROR_LEXICAL, "no such command ", token.content, token.value);
//		return 1;
//	}
//	else if(token.type == DISLEXER_TOKEN_TYPE_NUMBER){
//		write_error(context, MSHELL_ERROR_SYNTAX, "invalid operation", 0, 0);
//		return 1;
//	}
//
//	int result = 0;
//	switch(token.value){
//		default:
//			if(token.value > shell_command_table_size){
//				return -1;
//			}
//			result = call_table[token.value].command(context, command);
//			break;
//		case MEMORY_SHELL_WORD_exit:
//			context->result_code = MSHELL_SUCCESS;
//			context->result_value = -1;
//			result = -1; break;
//	}
//
//	return result;
//}
//
////----------------utility functions----------------
//
//static void copy_string(char* buffer, char* content, int size){
//	for(int i = 0; i < size; buffer[++i] = 0){ buffer[i] = content[i]; }
//}
//
//static int expect_type(mshell_context* context, dislexer_token* token, int type, char* message, char* param, int param_size){
//	if(token->type == type){ return 1; }
//	switch(token->type){
//		case DISLEXER_TOKEN_TYPE_NUMBER:
//			write_error(context, MSHELL_ERROR_SYNTAX, message, param + 1, param_size - 1);
//			write_string(context, " [numeric literal]", 99);
//			return 0;
//		case DISLEXER_TOKEN_TYPE_WORD:
//			write_error(context, MSHELL_ERROR_SYNTAX, message, param + 1, param_size - 1);
//			write_string(context, " [reserved word]", 99);
//			return 0;
//		case DISLEXER_TOKEN_TYPE_STRING:
//			write_error(context, MSHELL_ERROR_SYNTAX, message, param + 1, param_size - 1);
//			write_string(context, " [string literal]", 99);
//			return 0;
//	}
//	return 1;
//}
//
//static memory_unit* find_unit(mshell_context* context, char* param, dislexer_token* token, int param_size, int allow_parent){
//	if(param_size == 0 || (token->type == 0 && token->value == 0)){
//		write_error(context, MSHELL_ERROR_SYNTAX, "invalid operation (empty unit name)", 0, 0);
//		return (memory_unit*)1;
//	}
//
//	memory_unit* found;
//	switch(token->type){
//		case DISLEXER_TOKEN_TYPE_NUMBER:
//			write_error(context, MSHELL_ERROR_SYNTAX, "invalid entry name: ", param + 1, param_size - 1);
//			write_string(context, " [numeric literal]", 99);
//			return (memory_unit*)1;
//		case DISLEXER_TOKEN_TYPE_WORD:
//			if(token->value != MEMORY_SHELL_WORD_dotdot || !allow_parent){
//				write_error(context, MSHELL_ERROR_SYNTAX, "invalid entry name: ", param + 1, param_size - 1);
//				write_string(context, " [reserved symbol]", 99);
//				return (memory_unit*)1;
//			}
//			found = md_fetch_parent(context->focus);
//			break;
//		case DISLEXER_TOKEN_TYPE_STRING:;
//			char clean_string[token->value + 1]; copy_string(clean_string, token->content, token->value);
//			found = md_fetch_path(context->focus, clean_string);
//			break;
//	}
//
//	return found;
//}
//
//static memory_unit* create_unit(mshell_context* context, char* param, dislexer_token* token, int param_size, int allow_parent, int size){
//	memory_unit* found = find_unit(context, param, token, param_size, allow_parent);
//	if(found == (memory_unit*)1){ return (memory_unit*)1; }
//	if(found){ write_error(context, MSHELL_ERROR_COMMAND, "an entry with this name already exists", 0, 0); return (memory_unit*)1; }
//
//	char clean_string[token->value + 1]; copy_string(clean_string, token->content, token->value);
//	void* block = md_alloc_path(context->focus, clean_string, size);
//
//	if(!block){
//		write_error(context, MSHELL_ERROR_COMMAND, "failed to allocate memory for entry", 0, 0);
//		return (memory_unit*)1;
//	}
//
//	return md_header(block);
//}
//
//typedef struct _token_list{
//	dislexer_token* token;
//	struct _token_list* next;
//}token_list;
//
//static int append_number(char* block, int limit, int offset, unsigned long long int number){
//	if(!number && offset < limit){ block[offset++] = number; }
//	for(; number && offset < limit; number >>= 8, offset++){
//		block[offset] = number & 0xff;
//	}
//	return offset;
//}
//
//static int append_string(char* block, int limit, int offset, char* str, int str_size){
//	for(int i = 0; i < str_size && str[i] && offset < limit; i++, offset++){
//		block[offset] = str[i];
//	}
//	return offset;
//}
//
//static int write_token_data_recursive(mshell_context* context, int offset, char* param, token_list* start, token_list* end){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//
//	if(token.type == DISLEXER_TOKEN_TYPE_WORD && token.value != MEMORY_SHELL_WORD_line_break){
//		write_error(context, MSHELL_ERROR_SYNTAX, "invalid value ", param + 1, param_size - 1);
//		write_string(context, " [reserved word]", 99);
//		return 0;
//	}
//
//	if(token.type == DISLEXER_TOKEN_TYPE_NUMBER || token.type == DISLEXER_TOKEN_TYPE_STRING){
//		token_list this = { &token, 0 };
//
//		if(!start){ start = &this; }
//		if(end){ end->next = &this; }
//
//		end = &this;
//
//		return write_token_data_recursive(context, offset, param + param_size, start, end);
//	}
//
//	//if reached here, then we are on the end of the line
//
//	memory_unit* target = context->focus;
//
//	int size = target->size;
//	char* block = md_data(target);
//
//	for(token_list* list = start; list && offset < size; list = list->next){
//		dislexer_token* data = list->token;
//
//		if(data->type == DISLEXER_TOKEN_TYPE_NUMBER){
//			offset = append_number(block, size, offset, data->value);
//			continue;
//		}
//		offset = append_string(block, size, offset, data->content, data->value);
//	}
//
//	return 1;
//}
//
////----------------command implementation----------------
//
//int _mshell_command_cd(mshell_context* context, char* param){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//	clear_buffer(context);
//
//	memory_unit* found = find_unit(context, param, &token, param_size, 1);
//	if(found == (memory_unit*)1){ return 1; }
//	if(!found){ write_error(context, MSHELL_ERROR_COMMAND, "no such entry ", token.content, token.value); return 1; }
//
//	context->focus = found;
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = 0;
//
//	return 0;
//}
//
//int _mshell_command_ls(mshell_context* context, char* param){
//	clear_buffer(context);
//	param = param + 0;//quick fix for unused param warning
//	int found = 0;
//
//	for(memory_unit* inner = context->focus->children; inner; inner = inner->next, found++){
//		if(found){ write_string(context, "\n", 2); }
//		write_string(context, inner->name, 16);
//		write_string(context, " - Total: ", 99);
//		write_number(context, md_recursive_size(inner));
//		if(inner->size){
//			write_string(context, " (Self: ", 99);
//			write_number(context, inner->size);
//			write_string(context, ")", 3);
//		}
//	}
//
//	if(!found){ write_string(context, "this directory is empty", 24); }
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = found;
//	return 0;
//}
//
//int _mshell_command_mkdir(mshell_context* context, char* param){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//	clear_buffer(context);
//
//	memory_unit* created = create_unit(context, param, &token, param_size, 0, 0);
//	if(created == (memory_unit*)1){ return 1; }
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = 0;
//
//	return 0;
//}
//
//int _mshell_command_touch(mshell_context* context, char* param){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//	clear_buffer(context);
//
//	dislexer_token token_2; int param_size_2 = dislexer_parse(&mshell_codex, param + param_size, &token_2);
//	if(!expect_type(context, &token_2, DISLEXER_TOKEN_TYPE_NUMBER, "invalid format for entry size ", param + param_size, param_size_2)){
//		return 1;
//	}
//
//	memory_unit* created = create_unit(context, param, &token, param_size, 0, token_2.value);
//	if(created == (memory_unit*)1){ return 1; }
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = 0;
//
//	return 0;
//}
//
//int _mshell_command_rm(mshell_context* context, char* param){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//	clear_buffer(context);
//
//	int is_recursive = token.type == DISLEXER_TOKEN_TYPE_WORD && token.value == MEMORY_SHELL_WORD_r;
//	if(is_recursive){ param_size = dislexer_parse(&mshell_codex, param + param_size, &token); }
//
//	memory_unit* found = find_unit(context, param, &token, param_size, 0);
//	if(found == (memory_unit*)1){ return 1; }
//	if(!found){ write_error(context, MSHELL_ERROR_COMMAND, "no such entry ", token.content, token.value); return 1; }
//
//	if(found->children && !is_recursive){
//		write_error(context, MSHELL_ERROR_SYNTAX, "entry is not empty, use '-r' for recursive removal", 0, 0);
//		return 1;
//	}
//
//	md_free_unit(found);
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = 0;
//	return 0;
//
//	return 1;
//}
//
//int _mshell_command_hex(mshell_context* context, char* param){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//	clear_buffer(context);
//
//	if(token.type == DISLEXER_TOKEN_TYPE_WORD && token.value == MEMORY_SHELL_WORD_h){
//		write_string(context, "'hex' - reads bytes from the current focused entry\n", 99);
//		write_string(context, "'hex STRING' - reads bytes from the specified entry\n", 99);
//		write_string(context, "\n", 2);
//		write_string(context, "<-r NUMBER> - sets how many bytes will fit in each line\n", 99);
//		write_string(context, "<-l NUMBER> - sets how many lines will be read\n", 99);
//		write_string(context, "<-o NUMBER> - sets the offset in bytes");
//
//		context->result_code = MSHELL_SUCCESS;
//		context->result_value = 0;
//
//		return 0;
//	}
//
//	memory_unit* target = context->focus;
//	int range = 8;
//	int lines = 16;
//	int offset = 0;
//
//	if(token.type != 0 && token.value != 0){
//		if(token.type == DISLEXER_TOKEN_TYPE_STRING){
//			memory_unit* found = find_unit(context, param, &token, param_size, 1);
//			if(found == (memory_unit*)1){ return 1; }
//			if(!found){ write_error(context, MSHELL_ERROR_COMMAND, "no such entry ", token.content, token.value); return 1; }
//			target = found;
//
//			param += param_size;
//			param_size = dislexer_parse(&mshell_codex, param, &token);
//		}
//
//		while(token.type == DISLEXER_TOKEN_TYPE_WORD){
//			int word = token.value;
//
//			switch(word){
//				default:
//					write_error(context, MSHELL_ERROR_SYNTAX, "invalid parameter ", param + 1, param_size - 1);
//					write_string(context, " [reserved word]", 99);
//					return 1;
//				case MEMORY_SHELL_WORD_r: break;
//				case MEMORY_SHELL_WORD_l: break;
//				case MEMORY_SHELL_WORD_o: break;
//			}
//
//			param += param_size;
//			param_size = dislexer_parse(&mshell_codex, param, &token);
//
//			expect_type(context, &token, DISLEXER_TOKEN_TYPE_NUMBER, "invalid parameter ", param, param_size);
//
//			switch(word){
//			default:
//
//				case MEMORY_SHELL_WORD_r://range
//
//			}
//		}
//
//		for(; token.type == DISLEXER_TOKEN_TYPE_WORD;){
//
//		}
//
//		if(token.type == DISLEXER_TOKEN_TYPE_NUMBER){ range = token.value; }
////		else{
////			memory_unit* found = find_unit(context, param, &token, param_size, 1);
////			if(found == (memory_unit*)1){ return 1; }
////			if(!found){ write_error(context, MEMORY_SHELL_ERROR_COMMAND, "no such entry ", token.content, token.value); return 1; }
////
////			context->focus = found;
////			int result = _mshell_command_hex(context, param + param_size);
////			context->focus = target;
////			return result;
////		}
//	}
//
//	char* block = md_data(target);
//	int size = target->size;
//	for(int i = 0; i < size; i += range){
//		int ipr = i + range;
//		if(ipr >= size){ ipr = size; }
//
//		if(i){ write_string(context, "\n", 2); }
//		write_hex(context, i, 8); write_string(context, " | ", 4);
//
//		for(int l = i; l < ipr; l++){
//			if(l > i){ write_string(context, " ", 2); }
//			write_hex(context, block[l] & 0xff, 2);
//		}
//
//		for(int l = i + range - size; l > 0; l--){
//			write_string(context, " __", 4);
//		}
//
//		write_string(context, " | ", 4);
//
//		for(int l = i; l < ipr; l++){
//			int c = block[l];
//			if(c < 32 || c > 127){ c = '.'; }
//			write_char(context, c);
//		}
//	}
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = 0;
//
//	return 0;
//}
//
//int _mshell_command_stat(mshell_context* context, char* param){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//	clear_buffer(context);
//
//	memory_unit* target = context->focus;
//	if(token.type != 0 && token.value != 0){
//		memory_unit* found = find_unit(context, param, &token, param_size, 1);
//		if(found == (memory_unit*)1){ return 1; }
//		if(!found){ write_error(context, MSHELL_ERROR_COMMAND, "no such entry ", token.content, token.value); return 1; }
//
//		context->focus = found;
//		int result = _mshell_command_stat(context, param + param_size);
//		context->focus = target;
//		return result;
//	}
//
//	write_string(context, "ID: ", 99); write_hex(context, target->id, 16);
//	write_string(context, "\nName: ", 99); write_string(context, target->name, 16);
//
//	memory_unit* parent = md_fetch_parent(target);
//	if(parent){ write_string(context, "\nParent: ", 99); write_string(context, parent->name, 16); }
//
//	write_string(context, "\nMetadata: ", 99); write_hex(context, target->meta, 8);
//
//	write_string(context, "\nLocal size: ", 99); write_number(context, target->size);
//
//	if(target->children){
//		write_string(context, "\nTotal size: ", 99); write_number(context, md_recursive_size(target));
//		write_string(context, "\nDependents: ", 99); write_number(context, md_recursive_count(target) - 1);
//	}
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = 0;
//
//	return 0;
//}
//
//int _mshell_command_set(mshell_context* context, char* param){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//	clear_buffer(context);
//
//	memory_unit* target = context->focus;
//	int offset = 0;
//
//	if(token.type != 0 && token.value != 0){
//		if(token.type == DISLEXER_TOKEN_TYPE_NUMBER){ offset = token.value; }
//		else{
//			memory_unit* found = find_unit(context, param, &token, param_size, 1);
//			if(found == (memory_unit*)1){ return 1; }
//			if(!found){ write_error(context, MSHELL_ERROR_COMMAND, "no such entry ", token.content, token.value); return 1; }
//
//			context->focus = found;
//			int result = _mshell_command_set(context, param + param_size);
//			context->focus = target;
//			return result;
//		}
//	}
//
//	if(!write_token_data_recursive(context, offset, param + param_size, 0, 0)){
//		return 1;
//	}
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = 0;
//
//	return 0;
//}
//
//int _mshell_command_mem(mshell_context* context, char* param){
//	dislexer_token token; int param_size = dislexer_parse(&mshell_codex, param, &token);
//	clear_buffer(context);
//
//	if(param_size != 0 || (token.type != 0 && token.value != 0)){
//		if(!expect_type(context, &token, DISLEXER_TOKEN_TYPE_WORD, "invalid argument ", param, param_size)){ return 1; }
//		if(token.value == MEMORY_SHELL_WORD_merge){
//			s_merge();
//			write_string(context, "[MERGED] ", 99);
//		}
//	}
//
//	write_string(context, "System : ", 99);
//	write_number(context, static_memory_size);
//
//	write_string(context, " - ", 99);
//
//	write_string(context, "[Used : ", 99);
//	write_number(context, static_memory_size - avmem_global);
//	write_string(context, "]", 99);
//
//	write_string(context, " - ", 4);
//
//	write_string(context, "[Fragmented : ", 99);
//	write_number(context, avmem_global - avmem_rover);
//	write_string(context, "]", 99);
//
//	context->result_code = MSHELL_SUCCESS;
//	context->result_value = 0;
//
//	return 0;
//}
