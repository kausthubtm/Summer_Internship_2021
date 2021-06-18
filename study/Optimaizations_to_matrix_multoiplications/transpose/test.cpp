/******************************************************************************
* Matrix multiplication
******************************************************************************/

#include <omp.h>
#include <iostream>
using namespace std;

#define N 7

int a[N][N], b[N][N], c[N][N];

int main(int argc, char *argv[])
{
    int tid, nthreads, i, j, k, chunk;
    double start_time, run_time;
    chunk = 1;

    #pragma omp parallel shared(a,b,c,chunk,nthreads) private(tid,i,j,k)
    {
        #pragma omp for schedule (static, chunk)
            for(i=0; i<N; i++)
                for(j=0; j<N; j++)
                    a[i][j] = i+j;
        #pragma omp for schedule (static, chunk)
            for(i=0; i<N; i++)
                for(j=0; j<N; j++)
                   c[i][j] = i*j;
        #pragma omp for schedule (static, chunk)
            for(i=0; i<N; i++)
                for(j=0; j<N; j++) {
                    b[i][j] = c[j][i];
                    c[j][i] = 0;
                }


        start_time = omp_get_wtime();

        #pragma omp for schedule (dynamic, chunk)
        for(i=0; i<N; i++)
        {
            for(j=0; j<N; j++)
                for(k=0; k<N; k++)
                    c[i][j] += a[i][k]*b[j][k]; 
        }

        run_time = omp_get_wtime() - start_time;
    }


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