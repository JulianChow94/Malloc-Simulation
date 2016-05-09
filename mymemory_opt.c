#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define CHUNK_SIZE sizeof(struct block)
#define SYSTEM_MALLOC 1


// global variables
void *base = NULL;              // used to keep track of current address of heap
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct block *chunk;
struct block
{
    size_t size;                // size of the block
    size_t free;                // free space of the block
    chunk previous;
    chunk next;                 // pointer to next block
    char data[1];               // use for arithmetic
};

struct list                     // head of the list
{
    struct block *head;
};

struct list *L;

// Function to find free chunk of memory
// retval: a chunk of memory that satisfies size or 
// NULL if we couldn't find one
chunk find_chunk(chunk *last, size_t size)
    {   
        chunk current = base;         // current heap address           

        // while the heap address exists and
        // the current chunk isn't big enough to satisfy the requested size
        // we iterate through the list of chunks until we find one
        while (current && !(current->free && current->size >= size))
        {
            *last = current;
            current = current->next;
        }
        return current;
    }

// Function to use sbrk to grow the heap for when we can't find a chunk 
// to satisfy our arguement in mymalloc
// last is the current last chunk of the heap 
// and we set it's next value to the new chunk
// and set it's free space value to 0
// retval: return NULL is sbrk fails, return pointer to start of the new chunk
chunk extend(chunk last, size_t size)
    {

        chunk new_chunk = sbrk(0);      // create a new chunk of memory at current PB

        // move PB up enough for header and requested space
        if (sbrk(CHUNK_SIZE + size) == (void*)-1)
        {
            return NULL;
        }
        
        new_chunk->size = size;         // size is the requested size
        new_chunk->next = NULL;         // new chunk is the newest chunk         

        if (last != NULL)               // Assuming last isn't null, won't matter if NULL
            last->next = new_chunk;     // last now links with the new end of heap
            new_chunk->previous = last;
        new_chunk->free = 0;            // since chunk now allocated, so no free space

        return new_chunk;               // return address of the new chunk (where it starts)

    }

// Function to cut up block passed in to make a data block of the exact size
void split(chunk block, size_t size)
{
    chunk newblock;
    newblock = block->data + size;           // Pointer arithmetic to move where we are in heap

    newblock->size = block->size - size - CHUNK_SIZE;
    newblock->next = block->next;            // so that we can insert into the list
    newblock->free = 1;
    block->size = size;
    block->next = newblock;                  // put the old block BEHIND the new block 
}





/* mymalloc: allocates memory on the heap of the requested size. The block
             of memory returned should always be padded so that it begins
             and ends on a word boundary.
     unsigned int size: the number of bytes to allocate.
     retval: a pointer to the block of memory allocated or NULL if the 
             memory could not be allocated. 
             (NOTE: the system also sets errno, but we are not the system, 
                    so you are not required to do so.)
*/
void *mymalloc(unsigned int size) 
{
    chunk block;
    chunk last;                             // Keep track of last chunk
    

    /* Case where base is not initialzied and we don't know where PB is */
    pthread_mutex_lock(&lock);
    if (base == NULL)
    {
        block = extend(NULL, size);         // extend the heap
        base = block;                       // base is now the location of the first block
        L = base;
        L->head = block;
        if (!block)
            return NULL;                  
    }

    /* Case where we already know current PB */
    if (base)
    {
        last = base;                        // last is the current location of PB
        block = find_chunk(&last, size);    // find chunk and set block to that chunk

        if (block == NULL)                  // if we couldn't find a block (returns NULL)
            block = extend(last, size);     // we have to extend to heap
            if (!block)
                return NULL;
        if (block)
            // Here check our condition to see if we can split
            if ((block->size - size) >= (CHUNK_SIZE + 4))
                split(block, size);
        block->free = 0;                    // Unlike phase 1, we have a perfectly fitted block

    }
    pthread_mutex_unlock(&lock);
    return block;

} // end mymalloc

/* myfree: unallocates memory that has been allocated with mymalloc.
     void *ptr: pointer to the first byte of a block of memory allocated by 
                mymalloc.
     retval: 0 if the memory was successfully freed and 1 otherwise.
             (NOTE: the system version of free returns no error.)
*/
unsigned int myfree(void *ptr) 
{
	pthread_mutex_lock(&lock);
	chunk current = L->head;                            // get the first block of the list
    while (current != ptr)
    {
        current = current->next;
        //printf("Current block size:%d\n", current->size);
        //printf("Current free: %d\n", current->free);
    }

    if (current)
    {
        if (current->previous->free == current->previous->size)
        {
            current = current->previous;
            current->size += CHUNK_SIZE + current->next->size;
            current->free = current->size;
            pthread_mutex_unlock(&lock);
            return 0;
        }


        else
            current->free = current->size;
        
        pthread_mutex_unlock(&lock);

        return 0;
    }

    
    
    pthread_mutex_unlock(&lock);
    return 1;



   
    
    

    

	
}