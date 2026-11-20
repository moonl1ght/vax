#ifndef ADD_CUH
#define ADD_CUH

void launchAddInplace(
    float* __restrict__ A,
    const float* __restrict__ B,
    int A_size, // number of elements in A should be equal to or larger than B_size as we are adding B to A
    int B_size,
    int threadsPerBlock
);

#endif