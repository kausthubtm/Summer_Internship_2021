/******************************************************************************
* Matrix multiplication
******************************************************************************/

#include <omp.h>
#include <iostream>
using namespace std;

#define N 4

int a[N][N], b[N][N], c[N][N];

int main(int argc, char *argv[])
{
    int i, ii, j, jj, k, kk, B;
    double start_time, run_time;
    B = 2;

    start_time = omp_get_wtime();

    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
            a[i][j] = i+j;

    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
            b[i][j] = i*j;

    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
            c[i][j] = 0;
        
    for (ii = 0; ii < N; ii+=B) {
        for (kk = 0; kk < N; kk+=B) {
            for (jj = 0; jj < N; jj+=B) {
                for (i = ii; i < ii+B; i++) {
                    for (k = kk; k < kk+B; k++) {
                        for (j = jj; j < jj+B; j++) {
                            c[i][j] += a[i][k]*b[k][j];
                        }
                    }
                }
            }
        }
    }


    run_time = omp_get_wtime() - start_time;

    /*** Print results ***/
    printf("Compute Time: %f seconds\n", run_time);
    for(i=0;i<N;i++) {
        for(j=0;j<N;j++) {
            printf("%d ", c[i][j]);
        }
        printf("\n");
    }
    printf ("Done.\n");


}