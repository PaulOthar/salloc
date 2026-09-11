#include "memory_directory.h"

#ifdef MEMORY_DIRECTORY_SYSTEM_ALLOCATOR
#include <stdlib.h>
#define ALLOCATE_MEMORY(size) malloc(size)
#define FREE_MEMORY(ptr) free(ptr)
#define MERGE_AT(ptr)
#else
#include "static_allocator.h"
#define ALLOCATE_MEMORY(size) s_alloc(size)
#define FREE_MEMORY(ptr) s_free(ptr)
#define MERGE_AT(ptr) s_merge_at(ptr)
#endif

#ifdef MEMORY_DIRECTORY_DEBUG_MODE
#include <stdio.h>
#define MEMORY_DIRECTORY_DEBUG(MESAGE,...) printf("DEBUG <MD>: " MESAGE "\n", ##__VA_ARGS__)
#else
#define MEMORY_DIRECTORY_DEBUG(MESAGE, ...)
#endif

//https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t build_id(char* name){
	uint64_t result = 0xcbf29ce484222325ULL;

	for (int i = 0; i < 16 && name[i] && name[i] != '/'; i++) {
		result ^= name[i];
		result *= 0x100000001b3ULL;
	}

	return result;
}

static void copy_name(char* dest, char* src){
	int i = 0;
	for(; i < 15 && src[i] && src[i] != '/'; i++){
		dest[i] = src[i];
	}
	dest[i] = 0;
}

static char* skip_to_next(char* str){
	while(str[0] && str[0] != '/'){ str++; }
	return (str[0]) ? str + 1 : 0;
}

static memory_unit* find_near_unit(memory_unit* root, uint64_t id){
	for(; root; root = root->next){
		if(root->id == id){ return root; }
	}
	return 0;
}

static memory_unit* append_unit(memory_unit* dir, char* name, uint64_t id, int size){
	memory_unit* unit = (memory_unit*)ALLOCATE_MEMORY(size + sizeof(memory_unit));
	if(!unit){ return 0; }

	unit->size = size;

	unit->next = dir->children; if(unit->next){ unit->next->prev = unit; }
	unit->prev = dir;
	dir->children = unit;

	copy_name(unit->name, name);
	unit->id = id;

	unit->children = 0;

	unit->data = size ? (unit + 1) : 0;

	return unit;
}

//------------------------------------------------------------------------------------------------------------------------------------

void md_init_root(memory_unit* root){
	copy_name(root->name, "root");
	root->id = 0;

	root->next = 0;
	root->prev = 0;
	root->children = 0;

	root->size = 0;
}

void* md_alloc_path(memory_unit* current, char* path, int size){
	uint64_t id = build_id(path);
	memory_unit* found = current->children ? find_near_unit(current->children, id) : 0;
	char* next_path = skip_to_next(path);

	int condit = (found ? 1 : 0) | (next_path ? 2 : 0);
	switch(condit){
		case 0://there is no path with this name && there is no more ahead -> then this is what we came to create.
			found = append_unit(current, path, id, size);
			if(!found){ return 0; }
			MEMORY_DIRECTORY_DEBUG("%s > touch '%s' [%d B]\n", current->name, found->name, size);
			return found + 1;
		case 1://there is a path with this name && there is no more ahead -> the file already exists, dummy.
			MEMORY_DIRECTORY_DEBUG("%s > '%s' already exists", current->name, path);
			return 0;
		case 2://there is no path with this name && there is more ahead -> then create a directory and go to the next layer
			found = append_unit(current, path, id, 0);
			if(!found){ return 0; }

			MEMORY_DIRECTORY_DEBUG("%s > mkdir '%s'", current->name, found->name);
			MEMORY_DIRECTORY_DEBUG("%s > cd %s", current->name, found->name);

			void* result = md_alloc_path(found, next_path, size);
			if(!result){//something went wront, might as well free the allocated directory
				md_free_unit(found);
				MERGE_AT(found);
			}
			return result;
		case 3://there is a path with this name && there is more ahead -> then we found an existing known folder, go to the next layer.
			MEMORY_DIRECTORY_DEBUG("%s > cd %s", current->name, found->name);
			return md_alloc_path(found, next_path, size);
	}

	return 0;
}

void md_free_unit(memory_unit* unit){
	if(!unit){ return; }

	//Nuke all its children
	for(memory_unit* content = unit->children; content; content = content->next){
		md_free_unit(content);
	}
	MEMORY_DIRECTORY_DEBUG("Freeing unit '%s'", unit->name);

	if(unit->next){ unit->next->prev = unit->prev; }
	if(unit->prev){
		if(unit->prev->children == unit){ unit->prev->children = unit->next; }
		else{ unit->prev->next = unit->next; }
	}

	FREE_MEMORY(unit);
}

memory_unit* md_wrap_memory(memory_unit* current, char* path, int size, void* ptr){
	void* block = md_alloc_path(current, path, 0);
	memory_unit* wrapper = block ? md_header(block) : block;
	if(wrapper){
		wrapper->size = size;
		wrapper->data = ptr;
	}
	return wrapper;
}

memory_unit* md_fetch_path(memory_unit* root, char* path){
	for(char* p = path; p && root; p = skip_to_next(p)){
		if(!root->children){ return 0; }
		uint64_t id = build_id(p);
		root = find_near_unit(root->children, id);
	}
	return root;
}

memory_unit* md_fetch_parent(memory_unit* root){
	for(memory_unit* current = root->prev; root; current = root, root = root->prev){
		if(root->children == current){ return root; }
	}
	return root;
}

memory_unit* md_header(void* ptr){
	return (((memory_unit*) ptr) - 1);
}

void* md_data(memory_unit* unit){
	return unit->data;
}

int md_recursive_count(memory_unit* unit){
	int total = 1;
	for(memory_unit* child = unit->children; child; child = child->next){
		total += md_recursive_count(child);
	}
	return total;
}

int md_recursive_size(memory_unit* unit){
	int total = unit->size;
	for(memory_unit* child = unit->children; child; child = child->next){
		total += md_recursive_size(child);
	}
	return total;
}

//------------------------------------------------------------------------------------------------------------------------------------

static int name_len(char* name){
	int i = 0; while(name[i++]){} return i;
}

#include <stdio.h>

void md_print_tree(memory_unit* root, int identation){
	char identstr[identation + 1];
	for(int i = 0; i < identation; i++){ identstr[i] = ' '; }
	identstr[identation] = 0;

	const int sizeof_memory_unit = sizeof(memory_unit);

	for(; root; root = root->next){
		int size = root->size;
		char* name = root->name;

		printf("%s['%s'] -> size: %d (+ %d Header)\n", identstr, name, size, sizeof_memory_unit);
		if(root->children){ md_print_tree(root->children, identation + (name_len(name) / 2) + 2); }
	}
}
