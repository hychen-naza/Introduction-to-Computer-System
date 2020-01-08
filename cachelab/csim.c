#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "cachelab.h"

// simulate the line, set and entire cache
typedef struct{
  int valid;
  int tag;
  int lru;
}Line;

typedef struct{
  Line *lines;
  int nemptyl;
}Set;

typedef struct{
  Set *sets;
}Cache;

static struct option gLongOptions[] = {
  {"set_bits", required_argument, NULL, 's'},
  {"set_lines", required_argument, NULL, 'E'},
  {"block_bits", required_argument, NULL, 'b'},
  {"trace_file", required_argument, NULL, 't'},
  {NULL, 0, NULL, 0}
};

enum State{HIT, MISS, EVICT};

int NLINE = 1; // set line num
int BBITS = 1; // block bits num
int SBITS = 1; // set bits num
int nhit = 0;
int nmiss = 0;
int nevict = 0;
Cache scache;

int GetSetBits(int addr);
int GetTagBits(int addr);
void InitCache();
int FindLine(int setb, int tagb);
int UpdateLru(int setb, int tagb); 
int AddNewLine(int setb, int tagb); 
void UpdateStat(enum State res, char opt);

int main(int argc, char** argv)
{
  //char *file_path = (char*)malloc(sizeof(char)*20);
  int option_char = 0;
  char file_path[24];
  while((option_char = getopt_long(argc, argv, "s:E:b:t:", gLongOptions, NULL)) != -1){
    switch(option_char){
      default:
        fprintf(stderr, "input arguement not correct\n");
        exit(__LINE__);
      case 's':
        SBITS = atoi(optarg);
        break;
      case 'E':
        NLINE = atoi(optarg);
        break;
      case 'b':
        BBITS = atoi(optarg);
        break;
      case 't':
        strcpy(file_path, optarg);
        break;
    }
  }
  // check get-input-function
  // printf("s %d, E %d, b %d, track file %s\n", SBITS, NLINE, BBITS, file_path);
  // init cache
  InitCache();  

  FILE *fp = fopen(file_path, "r");
  char opt[2];
  int addr, size;
  while(fscanf(fp, "%s %x,%d", opt, &addr, &size) != EOF){
    if(opt[0] == 'I') continue;
    int setb = GetSetBits(addr);  
    int tagb = GetTagBits(addr);  
    //printf("GetBits Test: your input addr %x, set bits %x, tag bits %x\n", addr, setb, tagb);
    enum State res = FindLine(setb, tagb);
    //printf("find res %d, should be 1\n", res);
    if(res != HIT){
      if(AddNewLine(setb, tagb) == 1){
        printf("error in AddNewLine\n");
        exit(1);
      }
    }
    UpdateLru(setb, tagb); 
    UpdateStat(res, opt[0]);
    // debug print out
    /*printf("%s %x, ", opt, addr);
    if(res == HIT) printf("hit\n");
    if(res == MISS){
      if(opt[0] == 'M') printf("miss hit\n");
      else printf("miss\n");
    }
    if(res == EVICT){ 
      if(opt[0] == 'M') printf("miss eviction hit\n");
      else printf("miss eviction\n");
    }*/
  } 
  printSummary(nhit, nmiss, nevict);
  return 0;
}

// enum opt -> void
// update hit, miss, evict statis
void UpdateStat(enum State res, char opt){
  if(res == HIT) nhit++;
  else if(res == MISS) nmiss++;
  else if(res == EVICT){
    nmiss++;
    nevict++;
  }
  //printf("opt is %c\n", opt);
  if(opt == 'M') nhit++;
}

// init global struct scache
void InitCache(){
  int nset = 1 << SBITS; //
  scache.sets = (Set*)malloc(sizeof(Set)*nset);
  for(int i=0; i<nset; ++i){
    scache.sets[i].lines = (Line*)malloc(sizeof(Line)*NLINE);
    scache.sets[i].nemptyl = NLINE;
    for(int j=0; j<NLINE; ++j){
      scache.sets[i].lines[j].valid = 0;
    } 
  }
  //printf("your s input is %d, so we create %d sets\n", SBITS, nset); 
}

// int(hex form) -> int(hex form)
// get the set bits from addr
int GetSetBits(int addr){
  int mask = (1 << SBITS) - 1;
  int setb = (addr >> BBITS) & mask;
  return setb;
}

// int(hex form) -> int(hex form)
// get the tag bits from addr
int GetTagBits(int addr){   
   int mask = (1 << (SBITS+BBITS)) - 1;
   int tagb = (addr >> (SBITS+BBITS)) & mask;
   return tagb;
}


// int int -> int
// check if we can find this line in the set, return HIT, MISS, EVICT
int FindLine(int setb, int tagb){
  // we don't change set s in this function
  Set s = scache.sets[setb];
  for(int i=0; i<NLINE; ++i){
    if((s.lines[i].valid==1) && (s.lines[i].tag==tagb)){
      return HIT;
    }
  }
  if(s.nemptyl == 0) return EVICT;
  else if(s.nemptyl > 0) return MISS;
  else{
    printf("in find line func, set empty line number should not be negative\n");
    exit(1);
  }
}


// int -> int
// find the line with least lru in a set
int FindLeastLruLine(int setb){
  Set *s = &(scache.sets[setb]);
  int least_lru = NLINE+1;
  int index = 0;
  for(int i=0; i<NLINE; ++i){
    if(s->lines[i].valid==1 && s->lines[i].lru < least_lru){
      index = i; 
      least_lru = s->lines[i].lru;
    }
  }
  return index;
}


// int int -> int
// if we don't have HIT, add new line into set
int AddNewLine(int setb, int tagb){
  Set *s = &(scache.sets[setb]);
  if(s->nemptyl == 0){
    int line_index = FindLeastLruLine(setb);     
      //printf("not find evict line in set %d, line valid %d, lru %d\n", setb, s->lines[i].valid, s->lines[i].lru);
        //printf("find evict line in set %d, replace tag %d with tag %d\n", setb, s->lines[i].tag, tagb);
    s->lines[line_index].tag = tagb;
    s->lines[line_index].lru = NLINE;
    return 0;
  }
  else if(s->nemptyl > 0){
    for(int i=0; i<NLINE; ++i){
      if(s->lines[i].valid==0){
        //printf("find empty line in set %d\n", setb);
        s->lines[i].tag = tagb;
        s->lines[i].lru = NLINE;
        s->lines[i].valid = 1;
        s->nemptyl -= 1; 
        return 0;
      }
    }
  }
  printf("in add new line func, set empty line number should not be negative, value is %d\n", s->nemptyl);
  return 1;
}

// int int -> int
// update the LRU of all lines in the set
// I miss the case that lru should not reduce by 1 when it is already 1
// but it will also cause the situation that multi line lru are 1
// so, I allow the lru to be negative value in this sim cache(in reality, least lru is 1)
int UpdateLru(int setb, int tagb){
  Set *s = &scache.sets[setb];
  for(int i=0; i<NLINE; ++i){
    if(s->lines[i].valid==1){
      if(s->lines[i].tag == tagb) s->lines[i].lru = NLINE;
      else s->lines[i].lru -= 1;
    }
  }
  return 0;
}

