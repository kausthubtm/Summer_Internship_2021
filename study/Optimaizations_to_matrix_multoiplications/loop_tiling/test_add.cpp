#include <omp.h>
#include <iostream>
using namespace std;

#define N 4

int a[N][N], b[N][N], c[N][N];

int main(int argc, char *argv[])
{
    int tid, nthreads, i, ii, j, jj, k, kk, chunk, B;
    double start_time, run_time;
    B = 1;

    start_time = omp_get_wtime();

    #pragma omp parallel shared(a,b,c,chunk,nthreads,B) private(tid,i,j,k,ii,jj,kk)
    {
        #pragma omp for schedule (static, chunk)
            for(i=0; i<N; i++)
                for(j=0; j<N; j++)
                    a[i][j] = i+j;
        #pragma omp for schedule (static, chunk)
            for(i=0; i<N; i++)
                for(j=0; j<N; j++)
                   b[i][j] = i*j;
        #pragma omp for schedule (static, chunk)
            for(i=0; i<N; i++)
                for(j=0; j<N; j++)
                    c[i][j] = 0;
        
        for(k=0;k<1000;k++){
        #pragma omp for 
        for (ii = 0; ii < N; ii+=B) {
            for (jj = 0; jj < N; jj+=B) {
                for (i = ii; i < ii+B; i++) {
                    for (j = jj; j < jj+B; j++) {
                        c[i][j] = a[i][j] + b[i][j];
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
