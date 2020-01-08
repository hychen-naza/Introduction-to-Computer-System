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
#define CHUNKSIZE ((1 << 12) + 8)
//#define DEBUG 1
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define MAX(x, y) (((x) >= (y)) ? x : y)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
/* Pack the size and allocate bit into a word */
#define PACK(size, alloc) (size | alloc)
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


/* Data Representation */

void *free_list; /* Point to the first free block */


/* Defined Functions */

static void* extend_heap(size_t words);
static void insert_freelist(void *bp);
static void* find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void printinfo();
static void *coalesce(void *bp);
static void remove_freelist(void *bp);
static void printinfo(){
 
  //printf("in print info\n"); 
  void *p = free_list;
  while(p != NULL){
    printf("free block addr %p, size %d, next addr %p, prev addr %p\n", p, GET_SIZE(HDRP(p)), GET_SUCC(p), GET_PRED(p));
    p = GET_SUCC(p);
  }
}
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //printf("start init\n");
    free_list = NULL;
    void *heap_p;
    if((heap_p = mem_sbrk(4*WSIZE)) == (void *)-1) return -1;
    PUT(heap_p, 0);
    PUT((heap_p + WSIZE), PACK(DSIZE, 1));
    PUT((heap_p + (2*WSIZE)), PACK(DSIZE, 1));
    PUT((heap_p + (3*WSIZE)), PACK(0, 1));
    heap_p += 2*WSIZE; // might be useful in future
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) return -1;
    //printf("ok in init\n");
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
  //printf("start  malloc ??????????\n");
  if(size == 0) return NULL;

  size_t asize;
  size_t extendsize;
  void *bp;
  /* adjust block size to contain the head and foot */
  if(size <= DSIZE) asize = 2*DSIZE;
  else asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);
  if((bp = find_fit(asize)) != NULL){
    place(bp, asize);
    return bp;
  }

  extendsize = MAX(asize, CHUNKSIZE);
  
  if((bp = extend_heap(extendsize/WSIZE)) == NULL) return NULL;
  place(bp, asize);
  //printf("end  malloc ??????????\n");
  //sum_alloc += asize;
  //printf("total malloc size %d\n", sum_alloc);
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
  //printf("start free !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  insert_freelist(bp); 
  //printf("ok in free !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  coalesce(bp);
#ifdef DEBUG
    printinfo();
#endif
  //printf("ok in free\n");
}

/*
 * in realloc_place, bp is not in free_list, asize is the new block size
 */

static void realloc_place(void *bp, size_t asize){
  //printf("in place func\n");
  size_t old_size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(old_size, 1));
  PUT(FTRP(bp), PACK(old_size, 1));
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
  void *p = free_list; 
  while(p != NULL){
    if(GET_SIZE(HDRP(p)) >= asize){
      //printf("the suit free block size %d\n", GET_SIZE(HDRP(p)));
      return p;
    }
    p = GET_SUCC(p);
  }
  return NULL;
}

/*
 * insert a free block bp into free list
 */
static void insert_freelist(void *bp){ 
  //printf("in insert freelist\n");
  if(free_list == NULL){
    PUT_PRED(bp, NULL);
    PUT_SUCC(bp, NULL);
    free_list = bp;
  }
  else{
    int previous = 1; // means we should insert in previous place or succissive place 
    void *pos = free_list;
    //printf("insert block addr %p, current pos %p\n", bp, pos);
    // current free block address is larger or we have reached the end
    while(bp > pos){
      if(GET_SUCC(pos) == NULL){
        previous = 0;
        break;
      }
      pos = GET_SUCC(pos); 
    }
    // place bp before pos
    if(previous == 1){
      //printf("insert before pos, current bp addr %p, next addr %p, prev addr %p\n", bp, pos, GET_PRED(pos));
      PUT_PRED(bp, GET_PRED(pos));
      PUT_SUCC(bp, pos);
      if(GET_PRED(pos) != NULL){
        PUT_SUCC(GET_PRED(pos), bp);
      }
      else{
        free_list = bp;
      }
      PUT_PRED(pos, bp);
    }
    // place bp after
    else{
      //printf("insert after pos, bp addr %p, pos addr %p\n", bp, pos);
      PUT_PRED(bp, pos);
      PUT_SUCC(bp, GET_SUCC(pos));
      if(GET_SUCC(pos) != NULL) PUT_PRED(GET_SUCC(pos), bp);
      PUT_SUCC(pos, bp);
      //printf("4insert after pos, current bp addr %p, next addr %p, prev addr %p\n", bp, GET_SUCC(bp), GET_PRED(bp));
    }
  }
  //printf("out insert freelist\n"); 
/*#ifdef DEBUG
    printinfo();
#endif*/
}
/*
 * remove the free_block bp from the free list
 */
static void remove_freelist(void *bp){
  // the first free_block
  if(free_list == bp){
    void *next_freeb = GET_SUCC(bp);
    free_list = next_freeb;
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
  // the middle free block
  else{
    void *prev_freeb = GET_PRED(bp);
    void *next_freeb = GET_SUCC(bp);
    PUT_SUCC(prev_freeb, next_freeb);
    PUT_PRED(next_freeb, prev_freeb);
  }
  //printf("out remove freelist\n"); 
/*#ifdef DEBUG
    printinfo();
#endif*/
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
  //return bp;
  //printf("ok in extend\n");
  return coalesce(bp);
}


/*
 * use the free block and split this block if its size is larger than required
 */
static void place(void *bp, size_t asize){
  //printf("in place func\n");
  size_t old_size = GET_SIZE(HDRP(bp));
  if(old_size >= (asize + 2*DSIZE)){
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    PUT(HDRP(NEXT_BLKP(bp)), PACK((old_size - asize), 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK((old_size - asize), 0));
    /*instead of using remove_freelist and insert_freelist*/
    PUT_PRED(NEXT_BLKP(bp), GET_PRED(bp));
    PUT_SUCC(NEXT_BLKP(bp), GET_SUCC(bp)); 
    if(bp == free_list){
      free_list = NEXT_BLKP(bp); 
    }
    else{
      PUT_SUCC(GET_PRED(bp), NEXT_BLKP(bp));
    }
    if(GET_SUCC(bp) != NULL){
      PUT_PRED(GET_SUCC(bp), NEXT_BLKP(bp));
    }
    //remove_freelist(bp);
    //insert_freelist(NEXT_BLKP(bp));
  }
  else{
    PUT(HDRP(bp), PACK(old_size, 1));
    PUT(FTRP(bp), PACK(old_size, 1));
    remove_freelist(bp);
  }
}


static void *coalesce(void *bp){
  //printf("start coalesce\n");
  //printf("prev block address 0x%p", PREV_BLKP(bp));
  size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
  //printf("prev block alloc bit %d", prev_alloc);
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  //printf("next block alloc bit %d", next_alloc);
  size_t size = GET_SIZE(HDRP(bp));
  //printf("prev block alloc bit %d, next block alloc bit %d\n", prev_alloc, next_alloc);
  // can't coalescs
  if(prev_alloc && next_alloc){
    return bp;
  }
  // can coalescs the next block
  else if(prev_alloc && !next_alloc){
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    void *next_bp = GET_SUCC(GET_SUCC(bp));
    PUT_SUCC(bp, next_bp);
    if(next_bp != NULL) PUT_PRED(next_bp, bp); 
  }
  // can coalesce the previous block
  else if(!prev_alloc && next_alloc){
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    void *next_bp = GET_SUCC(bp);
    bp = PREV_BLKP(bp);
    PUT_SUCC(bp, next_bp);  
    if(next_bp != NULL) PUT_PRED(next_bp, bp);
  }
  else if(!prev_alloc && !next_alloc){
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    void *next_bp = GET_SUCC(GET_SUCC(bp));
    bp = PREV_BLKP(bp);
    PUT_SUCC(bp, next_bp);
    if(next_bp != NULL) PUT_PRED(next_bp, bp);
  }
  //printf("ok in coalesce\n");

/*#ifdef DEBUG
    printinfo();
#endif*/

  return bp;
}










