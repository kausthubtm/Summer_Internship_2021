#include <stdio.h>
#include <cuda.h>
__global__ void dkernel(){
    printf("Hi from thread id %d\n", threadIdx.x);
}
int main() {
    dkernel<<<1,32>>>();
    cudaDeviceSynchronize();
    return 0;
}