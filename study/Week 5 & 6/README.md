# Matrix Multiplication contd.

## Combo 
Loop Interchange + Loop Tiling
```
#pragma omp for
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
```
<br />

**RESULTS :**
<br />

Parallel implementation of combo

| Sl. No | Time |
| --- | --- |
| 1 | 7 min 8 sec |
| 2 | 7 min 23 sec |
| 3 | 7 min 8 sec |
| 4 | 7 min 9 sec |
| AVG | 7 min 12 sec |

<br />

Serial implementation of combo i.e., without '#pragma omp for'

| Sl. No | Time |
| --- | --- |
| 1 | 22 min 19 sec |
| 2 | 22 min 58 sec |
| 3 | 22 min 53  sec |
| 4 | 23 min 9 sec |
| AVG | 22 min 50 sec |

<br /><br />

## Naive Implementation on GPU using cuda

Source code : [here](https://colab.research.google.com/drive/1qbBkmM6oVZsD2QR8wMi_eTyH1Zhil2FD?usp=sharing)

<br />

**RESULTS :**
<br />

- All values are in micro seconds i.e., 10e-6

| Sl. No | Data transfer time from CPU - GPU | Cuda kernel time | Data transfer time from GPU - CPU |
| --- | --- | --- | --- |
| 1 | 3238516 | 25 | 4761718 |
| 2 | 3210122 | 21 | 4769050 |
| 3 | 3211761 | 20 | 4768124 |
| 4 | 3318618 | 23 | 4763245 |
| 5 | 3251083 | 21 | 4777266 | 
| 6 | 3209789 | 22 | 4760550 |
| AVG | 3.23 sec | 22 micro sec | 4.76 sec|

<br />
