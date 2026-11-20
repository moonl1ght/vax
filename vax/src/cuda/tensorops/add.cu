#include "add.cuh"
#include <cuda/cmath>

#define MAX_THREADS_PER_BLOCK 256

__global__ void add_kernel(
    float* __restrict__ A,
    const float* __restrict__ B,
    int A_size, // number of elements in A should be equal to or larger than B_size as we are adding B to A
    int B_size
) {
    // TODO: use grid stride loop
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int b_idx = idx % B_size;
    if (idx < A_size && b_idx < B_size) {
        A[idx] = A[idx] + B[b_idx];
    }
}

void launchAddInplace(
    float* __restrict__ A,
    const float* __restrict__ B,
    int A_size, // number of elements in A should be equal to or larger than B_size as we are adding B to A
    int B_size,
    int threadsPerBlock
) {
    int blocksPerGrid = cuda::ceil_div(A_size, threadsPerBlock);
    add_kernel<<<blocksPerGrid, threadsPerBlock>>>(A, B, A_size, B_size);
}