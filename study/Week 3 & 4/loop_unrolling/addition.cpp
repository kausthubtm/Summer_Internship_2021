/******************************************************************************
* Matrix multiplication
******************************************************************************/

#include <omp.h>
#include <iostream>
using namespace std;

#define N 10000

int a[N][N], b[N][N], c[N][N];

int main(int argc, char *argv[])
{
    int i, j, k;
    double start_time, run_time;

    start_time = omp_get_wtime();

    #pragma omp parallel shared(a,b,c) private(i,j,k)
    {
        #pragma omp for 
            for(i=0; i<N; i++)
                for(j=0; j<N; j++)
                    a[i][j] = i+j;
        #pragma omp for 
            for(i=0; i<N; i++)
                for(j=0; j<N; j++)
                   b[i][j] = i*j;
        #pragma omp for 
            for(i=0; i<N; i++)
                for(j=0; j<N; j++)
                    c[i][j] = 0;
        
        for(i=0; i<1000; i++){
            #pragma omp for 
            for(j=0; j<N; j++)
            {
                for(k=0; k<N; k=k+3) {
                    c[j][k] = a[j][k] + b[j][k];
                    c[j][k+1] = a[j][k+1] + b[j][k+1];
                    c[j][k+2] = a[j][k+2] + b[j][k+2];
                }
            }
        }
    }

    run_time = omp_get_wtime() - start_time;

    /*** Print results ***/
    printf("Compute Time: %f seconds\n", run_time);
    printf ("Done.\n");


}