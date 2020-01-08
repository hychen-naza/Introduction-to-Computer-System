/* 
 * CS:APP Data Lab 
 * 
 * <Name: Hongyi Chen  NetID: HCM5799>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * minusOne - return a value of -1 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int minusOne(void) {
  /*the bits pattern of -1 is 0xffffffff, which is ~0*/
  int res = ~0;
  return res;
}
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  /*if x is max, x+1 should be 0x80000000, x+(x+1) should be 0xffffffff,
  then x+(x+1)+1 should be 0x0, so use ! to this value we can get 1,
  but we need to exclude the situation when x is 1 through useing !add1x*/
  int add1x = x + 1;
  int all1 = x + add1x;
  int all0 = all1 + 1;
  int res = all0 + (!add1x);
  res = !res;
  return res;
}
/*
 * distinctNegation - returns 1 if x != -x.
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 5
 *   Rating: 2
 */
int distinctNegation(int x) {
  /*compute -x and compare x with -x*/
  int res = !!(x+x);
  return res;
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  /*if x,y are same signed, we should compute x-y and check if the result > 0.
  otherwise, if x is positive we can know x>y*/
  int signx = x >> 31;
  int signy = y >> 31;
  int samesign = (signx^signy);
  int isgreater = (x+(~y)) >> 31;
  int res = (!samesign & !isgreater) | (samesign & !signx);
  return res;
}
/* 
 * bitOr - x|y using only ~ and & 
 *   Example: bitOr(6, 5) = 7
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitOr(int x, int y) {
  /*reverse x,y to find the bits that are 0 in both x and y, 
  and then reverse (~x) & (~y) again we will know the result*/
  int res = ~((~x) & (~y));
  return res;
}
/* 
 * bitMatch - Create mask indicating which bits in x match those in y
 *            using only ~ and & 
 *   Example: bitMatch(0x7, 0xE) = 0x6
 *   Legal ops: ~ & |
 *   Max ops: 14
 *   Rating: 1
 */
int bitMatch(int x, int y) {
  /*x&y to find the matched 1bits, (~x)&(~y) to find the matech 0bits, combine them*/
  int res = (x&y) | ((~x)&(~y));
  return res;
}
/* 
 * anyOddBit - return 1 if any odd-numbered bit in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples anyOddBit(0x5) = 0, anyOddBit(0x7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int anyOddBit(int x) {
    /*create the oddmask to detect if we have any odd bit*/
    int oddmask = 0xaa;
    int oddmask2 = (oddmask << 8) | oddmask;
    int oddmask3 = (oddmask2 << 16) | oddmask2;
    int res = !!(x & oddmask3);
    return res;
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (least significant) to 3 (most significant)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  /*shift x based on the n*/
  int shifts = n << 3;
  int res = (x >> shifts) & 0xff;
  return res;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /*use signx and & to judge which to choose, y or z*/
  int signx = !x + (~0);
  int res = (signx & y) | (~signx & z);
  return res;
}
/*
 * isPallindrome - Return 1 if bit pattern in x is equal to its mirror image
 *   Example: isPallindrome(0x01234567E6AC2480) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 45
 *   Rating: 4
 */
int isPallindrome(int x) {
    /*reverse the low 16bits and compare it with high 16 bits*/
    int mask = (0xff << 8) | 0xff;
    int low16 = x & mask;
    int high16 = (x >> 16) & mask;
    // reverse
    int mask8 = 0xff;
    int mask4 = (0xf << 8) | 0xf;
    int mask2 = (0x33 << 8) | 0x33;
    int mask1 = (0x55 << 8) | 0x55;

    int high8 = (low16 >> 8) & mask8;
    int low8 = low16 << 8;
    int reverse8 = high8 | low8;

    int high4 = ((reverse8 >> 4)) & mask4;
    int low4 = (reverse8 & mask4) << 4;
    int reverse4 = (high4 | low4);

    int high2 = ((reverse4 >> 2)) & mask2;
    int low2 = (reverse4 & mask2) << 2;
    int reverse2 = (high2 | low2);

    int high1 = ((reverse2 >> 1)) & mask1;
    int low1 = (reverse2 & mask1) << 1;
    int reverse1 = (high1 | low1);
    // compare
    int res = !(reverse1 ^ high16);
    return res;
}
/* 
 * floatIsEqual - Compute f == g for floating point arguments f and g.
 *   Both the arguments are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   If either argument is NaN, return 0.
 *   +0 and -0 are considered equal.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 25
 *   Rating: 2
 */
int floatIsEqual(unsigned uf, unsigned ug) {
    /*do comparison according to different conditions*/
    int expuf = uf & 0x7f800000;
    int expug = ug & 0x7f800000;
    int fracuf = uf & 0x007fffff;
    int fracug = ug & 0x007fffff;
    int res;
    if(fracuf == 0 && fracug == 0){
        // +0 -0
        if(expuf == 0 && expug == 0) return 1; 
        // Infini
        res = !(uf ^ ug);
        return res;
    }
    // Nan
    //if(fracuf != 0 && expuf == 0x7f800000) return 0;
    //if(fracug != 0 && expug == 0x7f800000) return 0;
    if((uf & 0x7fffffff) > 0x7f800000) return 0;
    if((ug & 0x7fffffff) > 0x7f800000) return 0;
    res = !(uf ^ ug);
    return res;
}
/* 
 * floatScale1d2 - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale1d2(unsigned uf) {
  int s = uf & 0x80000000;
  int e = uf & 0x7f800000;
  // Nan, Infi
  if((uf & 0x7fffffff) >= 0x7f800000) return uf;
  // normal and will not become denormal after *0.5
  if(e > 0x800000) return (uf & 0x807fffff) | (e - 0x800000);
  // normal and will become denormal after *0.5
  // which means exp is 0x800000
  // we only need to move the exp value one bit right, 
  // the exps of original and res are identical (0), 
  // but the frac is reduced by twice

  // judge whether we need to do round before shifting
  // if the last two bits of uf is 0x3, we need to do round
  if((uf & 0x3) == 0x3) uf = uf + 0x2;
  return ((uf >> 1) & 0x007fffff) | s;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    /*check if x is in the range of float and then calculate if it is in*/
    unsigned res;
    if(x < -149) return 0;
    if(x > 127) return 0x7f800000;
    if(x >= -126) res = (x+127) << 23;
    else res = 1 << (23-(x+126)); 
    return res;
}
