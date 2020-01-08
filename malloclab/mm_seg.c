/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "naza",
    /* First member's full name */
    "Hongyi Chen",
    /* First member's email address */
    "HongyiChen2020@u.northwestern.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE ((1 << 12) + 48)
//#define DEBUG 1
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define MAX(x, y) (((x) >= (y)) ? x : y)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
/* Pack the size and allocate bit into a word */
#define PACK(size, alloc) ((size) | alloc)
/* Read and Write a word into address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = val)
/* Read the size and allocate bit from address p */
#define GET_SIZE(p) (GET(p) & (~0x7))
#define GET_ALLOC(p) (GET(p) & 0x1)
/* Given block ptr bp, computer the address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
/* Given block ptr bp, computer the address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)bp - DSIZE)))

/* Given block ptr p, put the pred and succ address */
/* Treat the address val as unsigned int*/
#define PUT_PRED(p, val) (*(unsigned int *)p = (unsigned int)val)
#define PUT_SUCC(p, val) (*(unsigned int *)((char *)p + WSIZE) = (unsigned int)val)
#define GET_PRED(p) ((void *)GET(p))
#define GET_SUCC(p) ((void *)GET(((char*)p + WSIZE)))

#define LISTLENGTH 9

/* Data Representation */

void *seg_free_list[LISTLENGTH];

/* Defined Functions */

static void* extend_heap(size_t words);
static void insert_freelist(void *bp);
static void* find_fit(size_t asize);
static void* place(void *bp, size_t asize);
static void printinfo();
static void *coalesce(void *bp);
static void remove_freelist(void *bp);
static int get_index(size_t size);


static void printinfo(){ 
  int i = 0;
  for(;i<LISTLENGTH;i++){
    void *p = seg_free_list[i];
    while(p != NULL){
      printf("free list %d, block addr %p, size %d, next addr %p, prev addr %p\n", i, p, GET_SIZE(HDRP(p)), GET_SUCC(p), GET_PRED(p));
      p = GET_SUCC(p);
    }
  }
}
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //printf("start init\n");
    void *heap_p;
    if((heap_p = mem_sbrk(4*WSIZE)) == (void *)-1) return -1;
    PUT(heap_p, 0);
    PUT((heap_p + WSIZE), PACK(DSIZE, 1));
    PUT((heap_p + (2*WSIZE)), PACK(DSIZE, 1));
    PUT((heap_p + (3*WSIZE)), PACK(0, 1));
    heap_p += 2*WSIZE; // might be useful in future
    int i=0;
    for(i=0;i<LISTLENGTH; ++i)
      seg_free_list[i] = NULL;
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) return -1;
#ifdef DEBUG
    printinfo();
#endif
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 * Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  //printf("start  malloc ??????????, print free block info\n");
  if(size == 0) return NULL;

  size_t asize;
  size_t extendsize;
  void *bp;
  /* adjust block size to contain the head and foot */
  if(size <= DSIZE) asize = 2*DSIZE;
  else asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);
  if((bp = find_fit(asize)) != NULL){
    bp = place(bp, asize);
#ifdef DEBUG
    printinfo();
#endif
    return bp;
  }
  extendsize = MAX(asize, CHUNKSIZE);
  
  if((bp = extend_heap(extendsize/WSIZE)) == NULL) return NULL;
  bp = place(bp, asize);
  //printf("end  malloc ??????????, start print info\n");
#ifdef DEBUG
    printinfo();
#endif
  return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
  size_t size = GET_SIZE(HDRP(bp));
  //printf("start free !!!!!!!!!!!!!!!!!!!!!!!!!!!!free size is %d, addr is %p\n", size, bp);
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  insert_freelist(bp); 
  coalesce(bp);
#ifdef DEBUG
    printinfo();
#endif
}

/*
 * in realloc_place, bp is not in free_list, asize is the new block size
 */

static void* realloc_place(void *bp, size_t asize){
  size_t old_size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(old_size, 1));
  PUT(FTRP(bp), PACK(old_size, 1));
  /* instead of split into smaller chunk, this simple method has higher space utilization */
  /*if(old_size >= (asize + 2*DSIZE)){
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    PUT(HDRP(NEXT_BLKP(bp)), PACK((old_size - asize), 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK((old_size - asize), 0));
    insert_freelist(NEXT_BLKP(bp));
  }
  else{
    PUT(HDRP(bp), PACK(old_size, 1));
    PUT(FTRP(bp), PACK(old_size, 1));
  }*/
}

/*
 * this new realloc_coalesce change the way of implementing the pointer to pred and succ
 */

static void *realloc_coalesce(void *bp, size_t newsize, int *coalnext){
  size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));
  // can coalescs the next block
  if(prev_alloc && !next_alloc){
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    if(size > newsize){
      remove_freelist(NEXT_BLKP(bp));
      PUT(HDRP(bp), PACK(size, 0));
      PUT(FTRP(bp), PACK(size, 0));
      *coalnext = 1;
    }
  }
  // can coalesce the previous block
  else if(!prev_alloc && next_alloc){
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    if(size >= newsize){
      remove_freelist(PREV_BLKP(bp));
      PUT(FTRP(bp), PACK(size, 0));
      PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
      bp = PREV_BLKP(bp);
    }
  }
  else if(!prev_alloc && !next_alloc){
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
    if(size >= newsize){
      remove_freelist(NEXT_BLKP(bp));
      remove_freelist(PREV_BLKP(bp));
      PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
      PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
      bp = PREV_BLKP(bp);
    }
  }
  return bp;
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  if(ptr == NULL)
    return mm_malloc(size);
  if(size == 0){
    mm_free(ptr);
    return NULL;
  }
  void *oldptr = ptr;
  void *newptr;
  size_t asize;  
  asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);    
  size_t old_size = GET_SIZE(HDRP(oldptr));
  if(old_size >= asize){
      realloc_place(ptr, asize);
      return ptr;
  }
  else{
      int coalnext = 0;
      newptr = realloc_coalesce(ptr, asize, &coalnext);
      if(coalnext == 1){
        realloc_place(ptr, asize);
        return ptr;
      }
      else if(coalnext == 0 && ptr != newptr){  
          memcpy(newptr, ptr, old_size - DSIZE);
          realloc_place(newptr, asize);
          return newptr;
      }
      else {
          newptr = mm_malloc(asize-DSIZE);
          memcpy(newptr, oldptr, old_size - DSIZE);
          mm_free(oldptr);
          return newptr;
      } 
  }
}


/*
 * find a suitable free block from the free list, we don't do split here
 */
static void* find_fit(size_t asize){
  int index = get_index(asize);
  for(;index<LISTLENGTH;++index){
    void *p = seg_free_list[index];
    while(p != NULL){
      if(GET_SIZE(HDRP(p)) >= asize){
        return p;
      }
      p = GET_SUCC(p);
    }
  }
  return NULL;
}

static int get_index(size_t size){
  if(size <= 32) return 0;
  else if(32 < size && size  <= 64) return 1;
  else if(64 < size && size <= 128) return 2;
  else if(128 < size && size <= 256) return 3;
  else if(256 < size && size <= 512) return 4;
  else if(512 < size && size <= 1024) return 5;
  else if(1024 < size && size <= 2048) return 6;
  else if(2048 < size && size <= 4096) return 7;
  else return 8;
}



/*
 * insert a free block bp into free list
 */
static void insert_freelist(void *bp){ 
  size_t size = GET_SIZE(HDRP(bp));
  //printf("in insert freelist, size is %d\n", size);
  int index = get_index(size);
  void *free_list = seg_free_list[index];
  if(free_list == NULL){
    PUT_PRED(bp, NULL);
    PUT_SUCC(bp, NULL);
    seg_free_list[index] = bp;    
  }
  else{
    int previous = 1; // means we should insert in previous place or succissive place 
    void *pos = free_list;
    size_t new_block_size = GET_SIZE(HDRP(bp));
    // PUT the block according to the block size
    while(new_block_size > GET_SIZE(HDRP(pos))){
      if(GET_SUCC(pos) == NULL){
        previous = 0;
        break;
      }
      pos = GET_SUCC(pos); 
    }
    // place bp before pos
    if(previous == 1){
      PUT_PRED(bp, GET_PRED(pos));
      PUT_SUCC(bp, pos);
      if(GET_PRED(pos) != NULL){
        PUT_SUCC(GET_PRED(pos), bp);
      }
      else{
        seg_free_list[index] = bp;
      }
      PUT_PRED(pos, bp);
    }
    // place bp after
    else{
      PUT_PRED(bp, pos);
      PUT_SUCC(bp, GET_SUCC(pos));
      if(GET_SUCC(pos) != NULL) PUT_PRED(GET_SUCC(pos), bp);
      PUT_SUCC(pos, bp);
    }
  }
#ifdef DEBUG
    printinfo();
#endif
}
/*
 * remove the free_block bp from the free list
 */
static void remove_freelist(void *bp){
  //printf("in remove freelist\n"); 
  size_t size = GET_SIZE(HDRP(bp));
  int index = get_index(size);
  void *free_list = seg_free_list[index];
  // the first free_block
  if(free_list == bp){
    void *next_freeb = GET_SUCC(bp);
    seg_free_list[index] = next_freeb;
    // maybe only one free block left
    if(next_freeb != NULL){
      PUT_PRED(next_freeb, NULL);
    }
  }
  // the last free block
  else if(GET_SUCC(bp) == NULL){
    void *prev_freeb = GET_PRED(bp); 
    // we need to add another condition to judge if it is the only free block
    if(prev_freeb != NULL) PUT_SUCC(prev_freeb, NULL);
  }
  // the middle free block, it must have a pred and succ
  else{
    void *prev_freeb = GET_PRED(bp);
    void *next_freeb = GET_SUCC(bp);
    PUT_SUCC(prev_freeb, next_freeb);
    PUT_PRED(next_freeb, prev_freeb);
  }
  //printf("out remove freelist, start print info\n"); 
#ifdef DEBUG
    printinfo();
#endif
}

/* 
 * create a new free_block with size words and put in into free list
 */
static void* extend_heap(size_t words){

  //printf("in extend heap\n");
  char *bp;
  size_t size = (words%2)? (words+1)*WSIZE : words*WSIZE; 
  if((void*)(bp = mem_sbrk(size)) == (void*)-1){
    return NULL;
  }
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
  insert_freelist(bp);
  return coalesce(bp);
}


/*
 * use the free block and split this block if its size is larger than required
 * return the place address
 */
static void* place(void *bp, size_t asize){
  //printf("in place func\n");
  size_t old_size = GET_SIZE(HDRP(bp));
  remove_freelist(bp);
  if(old_size < (asize + 2*DSIZE)){
    // we have to remove first, otherwise, its size will be changed and we can't remove correctly
    PUT(HDRP(bp), PACK(old_size, 1));
    PUT(FTRP(bp), PACK(old_size, 1));
    return bp;
  }
  /*else{
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    PUT(HDRP(NEXT_BLKP(bp)), PACK((old_size - asize), 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK((old_size - asize), 0));
    insert_freelist(NEXT_BLKP(bp));
  }*/
  // put into the tail
  else if(asize >= 96){
    PUT(HDRP(bp), PACK((old_size - asize), 0));
    PUT(FTRP(bp), PACK((old_size - asize), 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK((asize), 1));
    PUT(FTRP(NEXT_BLKP(bp)), PACK((asize), 1));
    insert_freelist(bp);
    return NEXT_BLKP(bp);
  }
  // put into the head
  else{
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    PUT(HDRP(NEXT_BLKP(bp)), PACK((old_size - asize), 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK((old_size - asize), 0));
    insert_freelist(NEXT_BLKP(bp));
    return bp;
  }
}


static void *coalesce(void *bp){
  //printf("start coalesce\n");
  size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));
  // can't coalescs
  if(prev_alloc && next_alloc){
    return bp;
  }
  // can coalescs the next block
  else if(prev_alloc && !next_alloc){
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    remove_freelist(NEXT_BLKP(bp));
    remove_freelist(bp);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    insert_freelist(bp);
  }
  // can coalesce the previous block
  else if(!prev_alloc && next_alloc){
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    remove_freelist(PREV_BLKP(bp));
    remove_freelist(bp);
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
    insert_freelist(bp);
  }
  else if(!prev_alloc && !next_alloc){
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
    remove_freelist(PREV_BLKP(bp));
    remove_freelist(NEXT_BLKP(bp));
    remove_freelist(bp);
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
    insert_freelist(bp);
  }
  //printf("ok in coalesce, start print free block info\n");

#ifdef DEBUG
    printinfo();
#endif

  return bp;
}










