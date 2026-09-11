#include <stdio.h>
#include "static_allocator.h"
#include "memory_shell.h"

char s_pool[1 << 26];

mshell_context context;
memory_unit root;
char command_buffer[1024];
char result_buffer[4096];

int main(){
	s_init(1 << 26);
	md_init_root(&root);
	mshell_context_init(&context, &root, result_buffer, 4096);

	md_wrap_memory(&root, "static/result", 4096, result_buffer);
	md_wrap_memory(&root, "static/command", 1024, command_buffer);

	while(context.result_value != -1){
		printf("%s> ", context.focus->name);
		fgets(command_buffer, 1024, stdin); mshell_run(&context, command_buffer); printf("%s%s", context.result_buffer, context.buffer_carriage ? "\n\n" : ""); continue;
//		mshell_run(&context, "cd static/command\n"); printf("%s%s", context.result_buffer, context.buffer_carriage ? "\n" : "");
//		mshell_run(&context, "stat -n\n"); printf("%s%s", context.result_buffer, context.buffer_carriage ? "\n" : "");
//		mshell_run(&context, "ls"); printf("%s%s", context.result_buffer, context.buffer_carriage ? "\n" : "");
		return 0;
	}
}
