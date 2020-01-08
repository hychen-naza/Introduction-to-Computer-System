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
#define CHUNKSIZE (1 << 12)
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

void *heap_p;
int sum_alloc = 0;


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
  }
  else if(!prev_alloc && next_alloc){
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }
  else if(!prev_alloc && !next_alloc){
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }
  //printf("ok in coalesce\n");
  return bp;
}


static void* extend_heap(size_t words){
  char *bp;
  size_t size = (words%2)? (words+1)*WSIZE : words*WSIZE; 
  if((void*)(bp = mem_sbrk(size)) == (void*)-1){
    return NULL;
  }
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
  //printf("ok in extend\n");
  return coalesce(bp);
}

static void* find_fit(size_t asize){
  void *p = heap_p; 
  while(GET_SIZE(HDRP((NEXT_BLKP(p)))) != 0){
    p = NEXT_BLKP(p);
    if(!GET_ALLOC(HDRP(p)) && GET_SIZE(HDRP(p)) >= asize){
      //printf("the suit free block size %d\n", GET_SIZE(HDRP(p)));
      return p;
    }
  }
  return NULL;
}

static void place(void *bp, size_t asize){

  size_t free_size = GET_SIZE(HDRP(bp));
  if(free_size >= (asize + 2*DSIZE)){
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    PUT(HDRP(NEXT_BLKP(bp)), PACK((free_size - asize), 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK((free_size - asize), 0));
  }
  else{
    PUT(HDRP(bp), PACK(free_size, 1));
    PUT(FTRP(bp), PACK(free_size, 1));
  }
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_p = mem_sbrk(4*WSIZE)) == (void *)-1) return -1;
    PUT(heap_p, 0);
    PUT((heap_p + WSIZE), PACK(DSIZE, 1));
    PUT((heap_p + (2*WSIZE)), PACK(DSIZE, 1));
    PUT((heap_p + (3*WSIZE)), PACK(0, 1));
    heap_p += 2*WSIZE; // might be useful in future
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) return -1;
    //printf("ok in init\n");

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  //printf("start  malloc\n");
  if(size == 0) return NULL;

  size_t asize;
  size_t extendsize;
  char *bp;
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
  //sum_alloc += asize;
  //printf("total malloc size %d\n", sum_alloc);
  return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  //printf("start free\n");
  size_t size = GET_SIZE(HDRP(ptr));
  //printf("you will free size %d\n", (int)size);
  PUT(HDRP(ptr), PACK(size, 0));
  PUT(FTRP(ptr), PACK(size, 0));
  coalesce(ptr);
  //printf("ok in free\n");
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    size_t asize;
    asize = DSIZE * ((size + (DSIZE-1)) / DSIZE);    
    newptr = mm_malloc(asize);
    if (ptr == NULL)
      return newptr;
    else{
      // printf("shift in realloc is %d\n", SIZE_T_SIZE);
      // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
      copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
      if (asize < copySize)
        copySize = asize;
      memcpy(newptr, oldptr, copySize);
      mm_free(oldptr);
      return newptr;
    }
}














