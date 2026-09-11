#ifndef STATIC_ALLOCATOR
#define STATIC_ALLOCATOR

extern int static_memory_size;
extern int avmem_global;
extern int avmem_rover;
extern char s_pool[];

#define STATIC_ALLOCATOR_INITIALIZE s_init(sizeof(s_pool));

/**
 * Initializes avmem_global and avmem_rover.
 * DOES NOT ALLOCATE SPACE FOR s_pool.
 * @fn void s_init(int)
 * @param mem_size The size in bytes
 */
void s_init(int mem_size);

/**
 * Allocates the specified number of bytes, with a possible additional cost of 8 bytes for the header.
 * Return null (0) if no big enough space is available.
 * @fn void s_alloc*(int)
 * @param size The size in bytes
 * @return a pointer to the allocated memory
 */
void* s_alloc(unsigned int size);

/**
 * Liberates the specific pointer's memory, making it available for reallocation, but beware of fragmentation.
 * This function will not automatically merge neighboring free blocks, unless if "STATIC_ALLOCATOR_MERGE_AFTER_FREEING" is defined.
 * This function will not check if it is pointing at a valid header, so tread carefully.
 * @fn void s_free(void*)
 * @param ptr Pointer to be freed
 */
void s_free(void* ptr);

/**
 * Merge will scan and combine neighboring regions of memory, if they are both free.
 * If it reaches the block neighboring the hover, it will effectively resize it to its combined size.
 * @fn void s_merge(void)
 */
void s_merge(void);

/**
 * Attempts to merge a specific block to its right neighbor.
 * @fn int s_merge_at(void*)
 * @param ptr To merge
 * @return 1 if it could merge, 0 if could not.
 *
 * @see s_merge
 */
int s_merge_at(void* ptr);

//------------------------------------------------------------------------------------------------------------------------------------

/**
 * Verifies if the pointer is within the memory pool range.
 * @fn int s_belongs(void*)
 * @param ptr Pointer to be verified
 * @return 1 if it is, 0 if it is not.
 */
int s_within(void* ptr);

/**
 * Verifies if the pointer has a known header.
 * @fn int s_validate(void*)
 * @param ptr Pointer to be verified
 * @return 1 if it is, 0 if it is not.
 */
int s_validate(void* ptr);

/**
 * Gets the size of this allocated block declared in the header.
 * @fn int s_size(void*)
 * @param ptr Pointer to be verified
 * @return size declared in the header
 */
int s_size(void* ptr);

/**
 * Verifies the block's header, if it is declared as used or free.
 * @fn int s_isfree(void*)
 * @param ptr Pointer to be verified
 * @return 1 if it is free, 0 if it is being used
 */
int s_isfree(void* ptr);

//------------------------------------------------------------------------------------------------------------------------------------

/**
 * Counts how many blocks there are. (free or not)
 * @fn int s_count(void)
 * @return how many exist.
 */
int s_count(void);

/**
 * Returns the address of the first block. (returns 0 if there is no block)
 * @fn void s_first*(void)
 * @return a pointer to the first block. (or 0 if there is no block)
 */
void* s_first(void);

/**
 * Returns the address of the next block. (returns 0 if there is no block ahead)
 * @fn void s_next*(void*)
 * @param ptr Current pointer
 * @return a pointer to the next blocks. (or 0 if it reached the end)
 */
void* s_next(void* ptr);

//------------------------------------------------------------------------------------------------------------------------------------

/**
 * Verifies if the specified block is valid before freeing, then free's it, if applicable.
 * @fn int s_free_safe(void*)
 * @param ptr Pointer to be verified / freed
 * @return 1 if it was freed successifully. 0 if not.
 */
int s_free_safe(void* ptr);

#endif
