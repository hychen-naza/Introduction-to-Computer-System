/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
  int r0, r1, r2, r3, r4, r5, r6, r7;
  if(M != 64){
    int i, k;
    if(M != 61){
      for(k = 0; k < M; k += 8){
        for(i=0; i < N; i++){
          r0 = A[i][k];
          r1 = A[i][k+1];
          r2 = A[i][k+2];
          r3 = A[i][k+3];
          r4 = A[i][k+4];
          r5 = A[i][k+5];
          r6 = A[i][k+6];
          r7 = A[i][k+7];
          B[k][i] = r0;
          B[k+1][i] = r1;
          B[k+2][i] = r2;
          B[k+3][i] = r3;
          B[k+4][i] = r4;
          B[k+5][i] = r5;
          B[k+6][i] = r6;
          B[k+7][i] = r7;
        }
      }
    }
    else{
      for(k = 0; k < 56; k += 8){
        for(i=0; i < N; i++){
          r0 = A[i][k];
          r1 = A[i][k+1];
          r2 = A[i][k+2];
          r3 = A[i][k+3];
          r4 = A[i][k+4];
          r5 = A[i][k+5];
          r6 = A[i][k+6];
          r7 = A[i][k+7];
          B[k][i] = r0;
          B[k+1][i] = r1;
          B[k+2][i] = r2;
          B[k+3][i] = r3;
          B[k+4][i] = r4;
          B[k+5][i] = r5;
          B[k+6][i] = r6;
          B[k+7][i] = r7;
        }
      }
      k = 56;
      for(i=0; i < N; i++){
          r0 = A[i][k];
          r1 = A[i][k+1];
          r2 = A[i][k+2];
          r3 = A[i][k+3];
          r4 = A[i][k+4];
          B[k][i] = r0;
          B[k+1][i] = r1;
          B[k+2][i] = r2;
          B[k+3][i] = r3;
          B[k+4][i] = r4;
      }
    }
  }
  else{
    /*int i, j, k, l, t1, t2, t3, t4, t5, t6, t7, t8;
    for (i = 0; i < N; i += 8) {
        for (j = 0; j < M; j += 8) {
            for (k = i; k < i + 4; k++) {
                t1 = A[k][j];
                t2 = A[k][j + 1];
                t3 = A[k][j + 2];
                t4 = A[k][j + 3];
                t5 = A[k][j + 4];
                t6 = A[k][j + 5];
                t7 = A[k][j + 6];
                t8 = A[k][j + 7];

                B[j][k] = t1;
                B[j + 1][k] = t2;
                B[j + 2][k] = t3;
                B[j + 3][k] = t4;
                B[j][k + 4] = t5;
                B[j + 1][k + 4] = t6;
                B[j + 2][k + 4] = t7;
                B[j + 3][k + 4] = t8;
            }
            for (l = j + 4; l < j + 8; l++) {

                t5 = A[i + 4][l - 4];
                t6 = A[i + 5][l - 4];
                t7 = A[i + 6][l - 4];
                t8 = A[i + 7][l - 4];

                t1 = B[l - 4][i + 4];
                t2 = B[l - 4][i + 5];
                t3 = B[l - 4][i + 6];
                t4 = B[l - 4][i + 7];

                B[l - 4][i + 4] = t5;
                B[l - 4][i + 5] = t6;
                B[l - 4][i + 6] = t7;
                B[l - 4][i + 7] = t8;

                B[l][i] = t1;
                B[l][i + 1] = t2;
                B[l][i + 2] = t3;
                B[l][i + 3] = t4;

                B[l][i + 4] = A[i + 4][l];
                B[l][i + 5] = A[i + 5][l];
                B[l][i + 6] = A[i + 6][l];
                B[l][i + 7] = A[i + 7][l];
            }
        }
    }*/




    int i, j, k, l;
    for(i=0; i < N; i += 8){
      for(j = 0; j < M; j += 8){
        for(k=i; k<i+4; ++k){
          r0 = A[k][j];
          r1 = A[k][j+1];
          r2 = A[k][j+2];
          r3 = A[k][j+3];
          r4 = A[k][j+4];
          r5 = A[k][j+5];
          r6 = A[k][j+6];
          r7 = A[k][j+7];
          B[j][k] = r0;
          B[j+1][k] = r1;
          B[j+2][k] = r2;
          B[j+3][k] = r3;
          B[j][k+4] = r4;
          B[j+1][k+4] = r5;
          B[j+2][k+4] = r6;
          B[j+3][k+4] = r7;
        }
        // the starting pos should change to j, end to j+4
        for(l=j+4; l<j+8; ++l){
          r4 = A[i+4][l-4];
          r5 = A[i+5][l-4];
          r6 = A[i+6][l-4];
          r7 = A[i+7][l-4];

          r0 = B[l-4][i+4];
          r1 = B[l-4][i+5];
          r2 = B[l-4][i+6];
          r3 = B[l-4][i+7];
  
          B[l-4][i+4] = r4;
          B[l-4][i+5] = r5;
          B[l-4][i+6] = r6;
          B[l-4][i+7] = r7;

          B[l][i] = r0;
          B[l][i+1] = r1;
          B[l][i+2] = r2;
          B[l][i+3] = r3;
  
          B[l][i+4] = A[i+4][l];
          B[l][i+5] = A[i+5][l];
          B[l][i+6] = A[i+6][l];
          B[l][i+7] = A[i+7][l];
        }
      }
    }
  }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

