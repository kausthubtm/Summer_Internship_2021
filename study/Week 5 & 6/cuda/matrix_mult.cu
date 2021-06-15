#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>
#include <chrono>

using namespace std::chrono;
using std::cout;
using std::generate;
using std::vector;

__global__ void matrixMul(const int *a, const int *b, int *c, int N) {
  // Compute each thread's global row and column index
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;

  // Iterate over row, and down column
  c[row * N + col] = 0;
  for (int k = 0; k < N; k++) {
    // Accumulate results for a single element
    c[row * N + col] += a[row * N + k] * b[k * N + col];
  }
}

// Check result on the CPU
void verify_result(vector<int> &a, vector<int> &b, vector<int> &c, int N) {
  // For every row...
  for (int i = 0; i < N; i++) {
    // For every column...
    for (int j = 0; j < N; j++) {
      // For every element in the row-column pair
      int tmp = 0;
      for (int k = 0; k < N; k++) {
        // Accumulate the partial results
        tmp += a[i * N + k] * b[k * N + j];
      }

      // Check against the CPU result
      assert(tmp == c[i * N + j]);
    }
  }
}

int main() {
    
  auto main_start = high_resolution_clock::now();  
  int N = 1 << 13;

  // Size (in bytes) of matrix
  size_t bytes = N * N * sizeof(int);

  auto start = high_resolution_clock::now();
  // Host vectors
  vector<int> h_a(N * N);
  vector<int> h_b(N * N);
  vector<int> h_c(N * N);

  
  // Initialize matrices
  generate(h_a.begin(), h_a.end(), []() { return rand() % 100; });
  generate(h_b.begin(), h_b.end(), []() { return rand() % 100; });

  
  // Allocate device memory
  int *d_a, *d_b, *d_c;
  cudaMalloc(&d_a, bytes);
  cudaMalloc(&d_b, bytes);
  cudaMalloc(&d_c, bytes);


  // Copy data to the device

  

  cudaMemcpy(d_a, h_a.data(), bytes, cudaMemcpyHostToDevice);
  cudaMemcpy(d_b, h_b.data(), bytes, cudaMemcpyHostToDevice);

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "Data transfer time from CPU - GPU : " << duration.count() << "\n";

  
  
  // Threads per CTA dimension
  int THREADS = 32;

  // Blocks per grid dimension (assumes THREADS divides N evenly)
  int BLOCKS = N / THREADS;

  // Use dim3 structs for block  and grid dimensions
  dim3 threads(THREADS, THREADS);
  dim3 blocks(BLOCKS, BLOCKS);

  
  
  

  // Launch kernel

  start = high_resolution_clock::now();

  matrixMul<<<blocks, threads>>>(d_a, d_b, d_c, N);

  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "cuda kernel time from CPU - GPU : " << duration.count() << "\n";




  // Copy back to the host

  start = high_resolution_clock::now();

  cudaMemcpy(h_c.data(), d_c, bytes, cudaMemcpyDeviceToHost);

  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "Data transfer time from GPU - CPU : " << duration.count() << "\n";

  // Check result
  // verify_result(h_a, h_b, h_c, N);

  // Free memory on device
  cudaFree(d_a);
  cudaFree(d_b);
  cudaFree(d_c);

  auto main_stop = high_resolution_clock::now(); 
  duration = duration_cast<microseconds>(main_stop - main_start); 
  cout << "Total time : " << duration.count() << "\n";

   cout << "COMPLETED SUCCESSFULLY\n";

  return 0;
}