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
        
        for(i=0; i<200; i++){
            #pragma omp for
            for(k=0; k<N; k++)
                for(j=0; j<N; j++) 
                    c[j][k] = a[j][k] + b[j][k];
        }
    }

    run_time = omp_get_wtime() - start_time;

    /*** Print results ***/
    printf("Compute Time: %f seconds\n", run_time);
    printf ("Done.\n");


}