#include "static_allocator.h"

#ifdef STATIC_ALLOCATOR_DEBUG_MODE
#include <stdio.h>
#define STATIC_ALLOCATOR_DEBUG(MESAGE,...) printf("DEBUG <SA>: " MESAGE "\n", ##__VA_ARGS__)
#else
#define STATIC_ALLOCATOR_DEBUG(MESAGE, ...)
#endif

#ifdef STATIC_ALLOCATOR_MERGE_AFTER_FREEING
#define STATIC_ALLOCATOR_AUTOMERGE s_merge();
#else
#define STATIC_ALLOCATOR_AUTOMERGE
#endif

#define STATIC_ALLOCATOR_TAG_FREE 0
#define STATIC_ALLOCATOR_TAG_USING 1

#define STATIC_ALLOCATOR_STEP_AHEAD(PTR,SIZE) (memheader*)(((void*)(PTR + 1)) + SIZE)

typedef struct memheader{
	unsigned int size;
	unsigned int tags;
}memheader;

int static_memory_size;
int avmem_global;
int avmem_rover;

#ifdef STATIC_ALLOCATOR_SIZE
char s_pool[STATIC_ALLOCATOR_SIZE];
#endif

static memheader* rover;
static const int sizemh = sizeof(memheader);

//__attribute__((constructor)) void static_allocator_initialize() {
//	for(int i = 0;i<STATIC_ALLOCATOR_SIZE;i++){ pool[i] = 0xff; }
//	avmem_global = STATIC_ALLOCATOR_SIZE - sizemh;
//	avmem_rover = avmem_global;
//	rover = (memheader*)pool;
//	rover->size = avmem_rover;
//	rover->tags = STATIC_ALLOCATOR_TAG_FREE;
//}

#define _BLOCK_HEADER(PTR) (((memheader*) PTR) - 1)

void s_init(int mem_size){
	static_memory_size = mem_size;
	avmem_global = mem_size - sizemh;
	avmem_rover = avmem_global;
	rover = (memheader*)s_pool;
	rover->size = avmem_rover;
	rover->tags = STATIC_ALLOCATOR_TAG_FREE;
}

void* s_alloc(unsigned int size){
	if(!size){ return 0; }
	#ifdef STATIC_ALLOCATOR_SIZE
		if(!rover){ s_init(sizeof(s_pool)); }
	#endif
	STATIC_ALLOCATOR_DEBUG("Attempting memory allocation of %d bytes from a total of %d", size, avmem_global);

	unsigned int sizereal = size + sizemh;
	memheader* result = (memheader*)s_pool;

	if(rover->size >= sizereal){//if the rover has enough space
		result = rover;
		//Step the bytes from the header and the size combined
		rover = (memheader*)(((void*)rover) + sizereal);
		rover->size = result->size - sizereal;
		rover->tags = STATIC_ALLOCATOR_TAG_FREE;

		result->size = size;
		result->tags = STATIC_ALLOCATOR_TAG_USING;

		avmem_global -= sizereal;
		avmem_rover = rover->size;

		STATIC_ALLOCATOR_DEBUG("    Allocating memory from rover: %d + %d (new header), Total = %d", size, sizemh, avmem_global);
		return (void*)(result + 1);
	}

	while(result != rover){//then starting from the beginning until we reach the rover

		//if this is not a free space, or it is free but it doesn't have enough space
		if(result->tags != STATIC_ALLOCATOR_TAG_FREE || result->size < size){
			result = (memheader*)(((void*)(result + 1)) + result->size);
			continue;
		}

		result->tags = STATIC_ALLOCATOR_TAG_USING;
		avmem_global -= result->size;

		STATIC_ALLOCATOR_DEBUG("    Allocating memory from reused header: %d, Total = %d)",result->size,avmem_global);

		//if there isn't any room for more than the header + 1 byte
		if(result->size <= sizereal){
			return (void*)(result + 1);//ship it with the current size (possibly greater)
		}

		//create a new header, with the size of the remaining bytes
		memheader* remainder = (memheader*)(((void*)(result + 1)) + size);
		remainder->size = result->size - sizereal;
		remainder->tags = STATIC_ALLOCATOR_TAG_FREE;
		avmem_global += remainder->size;

		STATIC_ALLOCATOR_DEBUG("    Resizing memory from reused header: %d (reused) - %d (new header) - %d (excess), Total = %d",result->size,sizemh,remainder->size,avmem_global);

		result->size = size;
		return (void*)(result + 1);
	}

	return 0;
}

void s_free(void* ptr){
	memheader* tofree = _BLOCK_HEADER(ptr);
	tofree->tags = STATIC_ALLOCATOR_TAG_FREE;
	avmem_global += tofree->size;
	STATIC_ALLOCATOR_DEBUG("Freeing memory: %d, Total: %d", tofree->size, avmem_global);
	STATIC_ALLOCATOR_AUTOMERGE
}

void s_merge(void){
	STATIC_ALLOCATOR_DEBUG("Attempting to merge unused headers | Total: %d", avmem_global);
	memheader* current = (memheader*)s_pool;
	while(current != rover){
		memheader* next = (memheader*)(((void*)(current + 1)) + current->size);

		if(current->tags != STATIC_ALLOCATOR_TAG_FREE || current->tags != next->tags){
			STATIC_ALLOCATOR_DEBUG("    Could not merge headers: [Size:%d, Tags:%d] X [Size:%d, Tags:%d] | Total: %d", current->size, current->tags, next->size, next->tags, avmem_global);
			current = next;
			continue;
		}

		avmem_global += sizemh;
		#ifdef STATIC_ALLOCATOR_DEBUG_MODE
		int pre_merge_size = current->size;
		#endif
		current->size += next->size + sizemh;
		STATIC_ALLOCATOR_DEBUG("    Merging headers: [Size:%d] + [Size:%d] + [Header:%d] = [Size:%d] | Total: %d", pre_merge_size, next->size, sizemh, current->size, avmem_global);

		if(next != rover){
			continue;
		}

		rover = current;
		avmem_rover = rover->size;
		STATIC_ALLOCATOR_DEBUG("    Header merged with rover: Total: %d, Rover: %d",avmem_global,avmem_rover);
		break;
	}
}

int s_merge_at(void* ptr){
	memheader* current = _BLOCK_HEADER(ptr);
	STATIC_ALLOCATOR_DEBUG("Attempting to merge target header of size %d", current->size);

	if(ptr == rover){
		STATIC_ALLOCATOR_DEBUG("    Merge canceled! (rover hit)");
		return 0;
	}

	memheader* next = (memheader*)(((void*)(current + 1)) + current->size);

	if(current->tags != STATIC_ALLOCATOR_TAG_FREE || current->tags != next->tags){
		STATIC_ALLOCATOR_DEBUG("    Merge canceled! (%s block is not free)", current->tags != STATIC_ALLOCATOR_TAG_FREE ? "current" : "next");
		return 0;
	}

	avmem_global += sizemh;
	#ifdef STATIC_ALLOCATOR_DEBUG_MODE
	int pre_merge_size = current->size;
	#endif
	current->size += next->size + sizemh;
	STATIC_ALLOCATOR_DEBUG("    Merging headers: [Size:%d] + [Size:%d] + [Header:%d] = [Size:%d] | Total: %d", pre_merge_size, next->size, sizemh, current->size, avmem_global);

	if(next == rover){
		rover = current;
		avmem_rover = rover->size;
		STATIC_ALLOCATOR_DEBUG("    Header merged with rover: Total: %d, Rover: %d",avmem_global,avmem_rover);
	}

	return 1;
}

//------------------------------------------------------------------------------------------------------------------------------------

#include <stdint.h>

int s_within(void* ptr){
	uintptr_t start = (uintptr_t)s_pool;
	uintptr_t end   = start + static_memory_size;
	uintptr_t addr  = (uintptr_t)ptr;

	int result = !(addr < start || addr >= end);

	STATIC_ALLOCATOR_DEBUG("Validating pointer range: <%s>\nStart\t0x%16X\nTarget\t0x%16x\nEnd\t0x%16X", result ? "True" : "False", start, addr, end);
	return result;
}

int s_validate(void* ptr){
	STATIC_ALLOCATOR_DEBUG("Verifying if the pointer <0x%X> has a known header...", (uintptr_t)ptr);

	memheader* target = _BLOCK_HEADER(ptr); uintptr_t target_addr  = (uintptr_t)target;
	memheader* head = (memheader*)s_pool;
	while(head != rover){
		uintptr_t head_addr = (uintptr_t)head;

		if(head_addr > target_addr){
			STATIC_ALLOCATOR_DEBUG("    Could not find a header with the specified address. (out of range)");
			return 0;
		}

		if((head_addr < target_addr) || (head != target)){ head = (memheader*)(((void*)(head + 1)) + head->size); continue; }

		STATIC_ALLOCATOR_DEBUG("    The header for the specified address is known.");
		return 1;
	}

	STATIC_ALLOCATOR_DEBUG("    Could not find a header with the specified address. (the rover was hit)");
	return 0;
}

int s_size(void* ptr){
	int result = _BLOCK_HEADER(ptr)->size;
	STATIC_ALLOCATOR_DEBUG("The declared size of the address <0x%X> is %d", (uintptr_t)ptr, result);
	return result;
}

int s_isfree(void* ptr){
	int result = _BLOCK_HEADER(ptr)->tags;
	STATIC_ALLOCATOR_DEBUG("The block of the address <0x%X> is currently %s.", (uintptr_t)ptr, result ? "in use" : "free");
	return !result;
}

//------------------------------------------------------------------------------------------------------------------------------------

int s_count(void){
	memheader* current = (memheader*)s_pool;
	int count = 0;
	STATIC_ALLOCATOR_DEBUG("Counting how many blocks there are...");
	while(current != rover){ current = (memheader*)(((void*)(current + 1)) + current->size); count++; }
	STATIC_ALLOCATOR_DEBUG("    %d blocks counted.", count);
	return count;
}

void* s_first(void){
	memheader* current = (memheader*)s_pool;
	STATIC_ALLOCATOR_DEBUG("Attempting to get the first block of the pool...");
	if(current == rover){
		STATIC_ALLOCATOR_DEBUG("    There is no block to get.");
		return 0;
	}
	STATIC_ALLOCATOR_DEBUG("    Found it!");
	return (void*)(current + 1);
}

void* s_next(void* ptr){
	memheader* this = _BLOCK_HEADER(ptr);
	memheader* next = (memheader*)(((void*)(this + 1)) + this->size);
	if(next == rover){ return 0; }
	return (void*)(next + 1);
}

//------------------------------------------------------------------------------------------------------------------------------------

int s_free_safe(void* ptr){
	if(!s_validate(ptr)){ return 0; }
	s_free(ptr);
	return 1;
}
