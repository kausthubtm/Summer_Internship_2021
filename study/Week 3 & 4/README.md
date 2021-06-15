# Matrix Multiplication

## Configuration 

The matrix multiplication was carried out using two 7000 X 7000 square matrices which was parallel run using OpenMP. Number of processors in the machine is 8 and L1d cache of 128Kb, L1i cache of 128Kb, L2 cache of 1Mb, and L3 cache of size 8Mb. The matrices was initialized as A[i][j] = i + j; and B[i][j] = i * j

## Naive Implementation

```
#pragma omp parallel for 
for(i=0; i<N; i++)
     for(j=0; j<N; j++)
          for(k=0; k<N; k++) 
               c[i][j] += a[i][k]*b[k][j];

```

On each iteration, the value of k is incremented. This means that in each iteration of the loop it is likely to suffer from a cache miss when loading the value of
b[k][j]. The reason for this is that because the matrix is stored in row-major order, each time you increment k, you're skipping over an entire row of the matrix
and jumping much further into memory, possibly far past the values you've cached. However, you don't have a miss when looking up c[i][j] (since i and j are the same)
, nor will you probably miss a[i][k], because the values are in row-major order and if the value of a[i][k] is cached from the previous iteration, the value of 
a[i][k] read on this iteration is from an adjacent memory location. Consequently, on each iteration of the innermost loop, you are likely to have one cache miss.

**RESULTS :**
<br />

| Sl. No | Time |
| --- | --- |
| 1 | 15 min 23 sec |
| 2 | 15 min 1 sec |
| 3 | 14 min 43 sec |
| 4 | 14 min 54 sec |
| 5 | 14 min 45 sec |
| 6 | 15 min 10 sec |
| 7 | 15 min 28 sec |
| AVG | 14 min 59 sec |

<br /><br />

## Loop Interchange
It is the process of exchanging the order of two iteration variables used by a nested loop. It is often done to ensure that the elements of a multidimensional 
array are accessed in the order in which they are present in memory, improving locality of reference.
<br />

```
#pragma omp parallel for 
for(i=0; i<N; i++)
     for(k=0; k<N; k++)
          for(j=0; j<N; j++) 
               c[i][j] += a[i][k]*b[k][j];

```

We are increasing j in every step instead of k. Since the values are stored in row major order c[i][j] is likely to be in cache, because the previous value of
c[i][j] from the previous iteration is likely to be cached as well and ready to be read. Similarly b[i][j] is probably cached. and since i and k aren't changing,
chances are a[i][k] is cached as well. This means that on each iteration of the inner loop, you're likely to have no cache misses.

**RESULTS :**
<br />

| Sl. No | Time |
| --- | --- |
| 1 | 7 min 2 sec |
| 2 | 7 min 18 sec |
| 3 | 7 min 2 sec |
| 4 | 7 min 2 sec |
| 5 | 7 min 2 sec |
| AVG | 7 min 5 sec |

<br /><br />

## Loop Unrolling
**Software pipilining :** Overlaps execution of instructions from multiple iterations of a loop and executes instructions from different iterations in the same 
pipeline, so that pipelines are kept busy without stalls. The objective is to sustain a high initiation rate [ Initiation of a subsequent iteration may start 
even before the previous iteration is complete]. <br />

**Loop Unrolling :** Loops are re-written as a replaced sequence of similar independent statements. This procedure mainly reduces loop overhead i.e., reduced 
branch instructions along with loop counter incrementation. On the other hand it increases the code length and readability and might cause reduction in 
performance.
 
```
#pragma omp parallel for 
for(i=0; i<N; i++)
    for(j=0; j<N; j++)
        for(k=0; k<N; k=k+4) {
            c[i][j] += a[i][k]*b[k][j];
            c[i][j] += a[i][k+1]*b[k+1][j];
            c[i][j] += a[i][k+2]*b[k+2][j];
            c[i][j] += a[i][k+3]*b[k+3][j];
         }

```
Mainly reduces loop overhead i.e., reduced branch instructions along with loop counter incrementation.

**RESULTS :**
<br />
- **4 times unrolled**

| Sl. No | Time |
| --- | --- |
| 1 | 15 min 2 sec |
| 2 | 14 min 45 sec |
| 3 | 14 min 33  sec |
| 4 | 14 min 34 sec |
| 5 | 14 min 48 sec |
| AVG | 14 min 44 sec |

- **4 times unrolled**

| Sl. No | Time |
| --- | --- |
| 1 | 15 min 35 sec |
| 2 | 15 min 5 sec  |
| 3 | 14 min 47 sec |
| 4 | 16 min 13 sec |
| 5 | 14 min 38 sec |
| AVG | 15 min 16 sec |

- **5 times unrolled**

| Sl. No | Time |
| --- | --- |
| 1 | 14 min 41 sec |
| 2 | 14 min 35 sec  |
| 3 | 14 min 48 sec |
| 4 | 14 min 39 sec |
| 5 | 14 min 28 sec |
| AVG | 14 min 38 sec |

<br /><br />

## Loop Tiling

The matrix is divided into smaller submatrices (tiles). The general idea of this technique is to change the loop’s iteration space into smaller blocks in order to 
work with the memory hierarchy more efficiently, i.e. we can guarantee that the data used in the loop will fit the cache until it is reused.


```
 #pragma omp parallel for         
 for (ii = 0; ii < N; ii+=B) 
      for (jj = 0; jj < N; jj+=B) 
           for (kk = 0; kk < N; kk+=B) 
               for (i = ii; i < ii+B; i++) 
                   for (j = jj; j < jj+B; j++) 
                        for (k = kk; k < kk+B; k++) 
                             c[i][j] += a[i][k]*b[k][j];

```

The matrix is broken down into BxB sub-matrices and the operations are performed on these sub-matrices. B is chosen in such a way that we can fit one block
from each matrix in the cache.

**RESULTS :**
<br />

- **Block size = 70 x 70**

| Sl. No | Time |
| --- | --- |
| 1 | 7 min 44 sec |
| 2 | 7 min 45 sec |
| 3 | 7 min 46 sec |
| 4 | 7 min 44 sec |
| 5 | 7 min 45 sec |
| AVG | 7 min 45 sec |

<br /><br />

## Transpose
```
for(i=0; i<N; i++)
    for(j=0; j<N; j++) {
        b[i][j] = c[j][i];
        c[j][i] = 0;
     }
         
 #pragma omp parallel for      
 for(i=0; i<N; i++)
     for(j=0; j<N; j++)
         for(k=0; k<N; k++)
             c[i][j] += a[i][k]*b[j][k]; 
```

After we perform the transpose operation on the b[][] matrix we can swap the ordering of the indices in ie., from b[k][j] -> b[j][k]. So, when k is incremented
we travel along the same row instead of the column and as values are stored in row major order, we could expect far lesser cache misses than traditional matrix
multiplication.

**RESULTS :**
<br />

| Sl. No | Time |
| --- | --- |
| 1 | 7 min 44 sec |
| 2 | 7 min 45 sec |
| 3 | 7 min 46 sec |
| 4 | 7 min 44 sec |
| 5 | 7 min 45 sec |
| AVG | 7 min 45 sec |

<br />


