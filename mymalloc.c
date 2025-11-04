#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

//----------
//Function Definitions
void* mymalloc (size_t size);
void myfree(void* ptr);

mblock_t* findLastMemlistBlock();
mblock_t* findFreeBlockOfSize(size_t size);
void splitBlockAtSize(mblock_t * block, size_t newSize);
void coallesceBlockPrev(mblock_t * freedBlock);
void coallesceBlockNext(mblock_t * freedBlock);
mblock_t* growHeapBySize(size_t size);

void printMemList(const mblock_t* headptr);
//----------
//Memory Block Definitions
typedef struct _mblock_t {
  struct _mblock_t * prev;
  struct _mblock_t * next;
  size_t size;
  int status;
  void * payload;
} mblock_t;

typedef struct _mlist_t {
    mblock_t * head;
} mlist_t;
//----------

mlist_t memoryList;

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload)

int main (int argc, char* argv[])
{
    //TODO: Test other sequences of events to ensure no errors occur!

    /*
    printMemList - Sets up a data structure that represents a doubly linked list of memory blocks.
        size: size of block in bytes
        status: whether block is allocated or free
        payload: pointer to actual usable memory
    */

    memoryList.head = NULL; //Initialize

    void * p1 = mymalloc(10);
    printMemList(memoryList.head);

    void * p2 = mymalloc(100);
    printMemList(memoryList.head);

    void * p3 = mymalloc(200);
    printMemList(memoryList.head);

    void * p4 = mymalloc(500);
    printMemList(memoryList.head);

    myfree(p3); p3 = NULL;
    printMemList(memoryList.head);

    myfree(p2); p2 = NULL;
    printMemList(memoryList.head);

    void * p5 = mymalloc(150);
    printMemList(memoryList.head);

    void * p6 = mymalloc(500);
    printMemList(memoryList.head);

    myfree(p4); p4 = NULL;
    printMemList(memoryList.head);

    myfree(p5); p5 = NULL;
    printMemList(memoryList.head);

    myfree(p6); p6 = NULL;
    printMemList(memoryList.head);

    myfree(p1); p1 = NULL;
    printMemList(memoryList.head);
    
    return 0;
}

void* mymalloc (size_t size)
{
    //Inside this implementation we should bookkeep virtual addresses between the bottom of the heap and the current location of the break that are free, with a Freelist data structure.
    //brk and sbrk: https://linux.die.net/man/2/brk - Increasing program break allocates memory to the process, decreasing the program break deallocates memory.
    //sbrk() increaments the program's data space by increment bytes. Calling sbrk with 0 can find the current location of the program break.
    //brk() On success returns 0, error is -1. Errno is set to ENOMEM.
    //sbrk() On success returns the previous program break (if the break was increased then this value is a pointer to the start of the newly allocated memory).

    //[mmap] (https://linux.die.net/man/2/mmap): This creates a Virtual Address mapping in the Process’ Virtual Address Space at the location indicated by the addr parameter,
    //and a size indicated by the length parameter. Creates a "Anonymous Memory region" in the program's Virtual Memory Space. Use the functionality of this system call to treat the mmap-ed
    //Anonymous Memory Region as Heap memory.
    //Deallocations of Virtual Address Space Regions corresponding to previously allocated objects can be accomplished via [munmap](https://linux.die.net/man/2/munmap).
    //Should return Pointer to Virtual Address location which was allocated and can be safely used (up-to size Bytes in length) and NULL if allocation failed.

    //Newly allocated data should be placed at MBLOCK_HEADER_SZ
    //Size of entire free block is mblock_t.size + MBLOCK_HEADER_SZ

    mblock_t firstFreeBlock = findFreeBlockOfSize(size);
    if (firstFreeBlock == NULL)
    { //There was no free memory for this size, so grow the heap.
        growHeapBySize(size);
    }

    return NULL;
}

void myfree(void* ptr)
{
    //Performs a basic santity check with respect to virtual memory bounds by checking whether the address of the block to be freed is
    //greater than the head of the Memorylist, and lesser than the current program break.
    if (ptr == NULL) { return; }
    if (ptr < (void*)memoryList.head || ptr >= (void*)sbrk(0))
    { 
        printf("Error, the pointer given was out of bounds!");
        return; 
    }
    
}

mblock_t* findLastMemlistBlock() //Traverses Memorylist to find the last Memory Block.
{
    mblock_t* currentBlock = memoryList.head;

    if (currentBlock == NULL) { return NULL; } //Empty memory list!
    
    while(currentBlock->next != NULL)
    {
        currentBlock = currentBlock->next;
    }
    return currentBlock;
}

mblock_t* findFreeBlockOfSize(size_t size) //Traverse Memorylist to find the first Free Memory Block that can fit a requested payload data size.
{
    mblock_t* currentBlock = memoryList.head;
    
    while(currentBlock != NULL)
    {
        if (currentBlock->status == 0 && size <= currentBlock->size)
        { //The current block has enough free space of the requested size.
            if (currentBlock->size > size + MBLOCK_HEADER_SZ)
            { //There is extra room split block into seperate allocated and unallocated memory blocks.
                splitBlockAtSize(currentBlock, size);
            }
            currentBlock->status = 1;
            return currentBlock;
        }
        currentBlock = currentBlock->next;
    }
    return NULL; //No free memory for required space.
}
//Split a MemoryBlock such that 2 Memory Blocks are created, the first one being able to accommodate a specific payload new size requested,
//and the second one getting whatever remains. Makes sure the Memorylist structure is kept up-to-date
void splitBlockAtSize(mblock_t * block, size_t newSize)
{
    mblock_t* nextBlock = block->next;

    mblock_t* remainingBlock = (mblock_t*)((char*)block + MBLOCK_HEADER_SZ + newSize);
    remainingBlock->size = block->size - newSize - MBLOCK_HEADER_SZ;
    remainingBlock->status = 0; //Free space
    remainingBlock->prev = block;
    remainingBlock->next = nextBlock;
    remainingBlock->payload = (void*)((char*)remainingBlock + MBLOCK_HEADER_SZ);

    block->size = newSize;
    block->next = remainingBlock;

    if (nextBlock != NULL) { nextBlock->prev = remainingBlock; }
}
//Coalesce a Memory Block with the previous one in the Memorylist (assuming it is Free). Makes sure the Memorylist structure is kept up-to-date.
void coallesceBlockPrev(mblock_t * freedBlock)
{

}
//Coalesce a Memory Block with the next one in the Memorylist (assuming it is Free). Makes sure the Memorylist structure is kept up-to-date.
void coallesceBlockNext(mblock_t * freedBlock)
{
 
}
//Increase the Heap allocation and create a new Memory Block in the Virutal Address Space which was reserved. Attach it to the Memorylist.
mblock_t* growHeapBySize(size_t size)
{
    size_t breakSize = max(size + MBLOCK_HEADER_SZ, 1000);
    void* prevBreakPoint = sbrk(breakSize); //Takes in increments in bytes for arguement.

    if (prevBreakPoint == (void*)-1)
    {
        printf("There was an error using sbrk(): %s\n", strerror(errno));
        return NULL;
    }

    mblock_t* lastBlock = findLastMemlistBlock();
    mblock_t* newBlock = (mblock_t*)prevBreakPoint;

    newBlock->size = breakSize - MBLOCK_HEADER_SZ;
    newBlock->next = NULL;
    newBlock->prev = lastBlock;
    newBlock->status = 0;
    newBlock->payload = (void*)((char*)newBlock + MBLOCK_HEADER_SZ);
    
    if (lastBlock != null)
    {
        lastBlock->next = newBlock;
    }
    else
    {
        memoryList.head = newBlock;
    }
    return newBlock;
}

size_t max(size_t a, size_t b)
{
    return (a > b) ? a : b;
}

void printMemList(const mblock_t* head) {
  const mblock_t* p = head;
  
  size_t i = 0;
  while(p != NULL) {
    printf("[%ld] p: %p\n", i, (void*)p);
    printf("[%ld] p->size: %ld\n", i, p->size);
    printf("[%ld] p->status: %s\n", i, p->status > 0 ? "allocated" : "free");
    printf("[%ld] p->prev: %p\n", i, (void*)p->prev);
    printf("[%ld] p->next: %p\n", i, (void*)p->next);
    printf("___________________________\n");
    ++i;
    p = p->next;
  }
  printf("===========================\n");
}