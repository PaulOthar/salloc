#ifndef MEMORY_DIRECTORY
#define MEMORY_DIRECTORY

#include <stdint.h>

typedef enum {
	MEMORY_DIRECTORY_STATIC = 1,
} md_flag;

typedef struct _memory_unit{
	char name[16];

	uint32_t size;
	uint64_t id;

	struct _memory_unit* prev;
	struct _memory_unit* next;
	struct _memory_unit* children;
	void* data;
}memory_unit;

/**
 * Initializes important fields in a memory_unit.
 * Usefull for when creating a memory_unit on the stack.
 * @fn void md_init_root(memory_unit*)
 * @param root Pointer of the unit to be initalized
 */
void md_init_root(memory_unit* root);

/**
 * Allocates a new unit and attaches it to the specified root, following the specific path.
 * Creates new directories if the specified path does not exist yet.
 * Each segment must be separated by a forward slash: '/'
 * @fn void md_alloc_path*(memory_unit*, char*, int)
 * @param root Pointer of the main unit
 * @param path Sequence of directory names separated by a forward slash '/'
 * @param size Size of the allocated size of the final unit
 * @return a pointer to the allocated memory
 */
void* md_alloc_path(memory_unit* root, char* path, int size);

/**
 * Liberates the specific pointer's (and its children) memory, making it available for reallocation. (prone to fragmentation)
 * If you free a directory, every subdirectory and 'file' will also be freed.
 * @fn void md_free_unit(memory_unit*)
 * @param unit Pointer of the unit to be freed
 */
void md_free_unit(memory_unit* unit);

/**
 * Wraps the specified pointer in a memory directory (puts it in the directory tree).
 * Creates new directories if the specified path does not exist yet.
 * Each segment must be separated by a forward slash: '/'
 * @fn memory_unit md_wrap_memory*(memory_unit*, char*, int, void*)
 * @param root Pointer of the main unit
 * @param path Sequence of directory names separated by a forward slash '/'
 * @param size Size of the memory block (in bytes)
 * @param ptr Pointer to the block
 * @return Pointer to the generated unit
 */
memory_unit* md_wrap_memory(memory_unit* root, char* path, int size, void* ptr);

/**
 * Finds and returns a `memory_unit` on the specified path.
 * @fn memory_unit md_fetch_path*(memory_unit*, char*)
 * @param root Pointer to the main unit
 * @param path Sequence of directory names separated by a forward slash '/'
 * @return a pointer to the unit found, or null (0) if none were found
 */
memory_unit* md_fetch_path(memory_unit* root, char* path);

/**
 * Finds and returns a `memory_unit` above.
 * @fn memory_unit md_fetch_parent*(memory_unit*)
 * @param root Pointer to the current unit
 * @return a pointer to the unit found, or null (0) if none were found
 */
memory_unit* md_fetch_parent(memory_unit* root);

//-----------------------------------------------------------------------------------------

/**
 * Reads the header of the specified block.
 * WARNING: if this block was not allocated by the directory system, it may cause segfault.
 * @fn memory_unit md_header*(void*)
 * @param ptr Pointer to block of memory
 * @return a pointer to the header of the specified block.
 */
memory_unit* md_header(void* ptr);

/**
 * Reads the block of the specified header.
 * @fn void md_data*(memory_unit*)
 * @param unit Pointer to the header
 * @return a pointer to the block of the specified header
 */
void* md_data(memory_unit* unit);

/**
 * Reads the count of the entire tree from the specified branch, recursively.
 * @fn int md_recursive_count(memory_unit*)
 * @param unit Pointer to the top unit
 * @return total count of branches and leafs the tree
 */
int md_recursive_count(memory_unit* unit);

/**
 * Reads the size of the entire tree from the specified branch, recursively.
 * @fn int md_recursive_size(memory_unit*)
 * @param unit Pointer to the top unit
 * @return total size of the tree
 */
int md_recursive_size(memory_unit* unit);

//-----------------------------------------------------------------------------------------

void md_print_tree(memory_unit* root, int identation);

#endif
