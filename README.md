# Construction Compilers  
This project implements a simple two-pass compiler optimizer designed to transform and optimize source code for better performance and maintainability.

## Optimization Passes  
### 1. Unused Argument Elimination
This pass analyzes each function in the input code and removes any arguments that are declared but never used within the function body. This helps reduce clutter and improves both readability and potential performance.

**Example:**
```c
// Before
int add(int a, int b, int unused) {
    return a + b;
}

// After
int add(int a, int b) {
    return a + b;
}
```

### 2. Polynomial Multiplication optimizer  
This pass replaces naive polynomial multiplication algorithms (typically O(n²)) with a Fast Fourier Transform (FFT)-based approach, significantly improving performance (to O(n log n)) for large polynomials.

**Example**
```c
// Before
void multiply_arrays1(int* A, int A_SIZE, int* B, int B_SIZE, int* C) {
    int result_size = A_SIZE + B_SIZE - 1;
    for(int i = 0; i < result_size; i++) 
        C[i] = 0;

    for(int i = 0; i < A_SIZE; i++) {
        for(int j = 0; j < B_SIZE; j++) {
            C[i + j] += A[i] * B[j];
        }
    }
}

// After
void multiply_arrays1(int* A, int A_SIZE, int* B, int B_SIZE, int* C) {
    // Here calling FFT algorithm (check FFTTransform directory for examples and explanation)
}
```

**or**  
```c
// Before
int* multiply_arrays2(int* A, int A_SIZE, int* B, int B_SIZE) {
    int result_size = A_SIZE + B_SIZE - 1;
    int* C = malloc(result_size * sizeof(int));
    if(!C) 
        return NULL;

    for(int i = 0; i < result_size; i++) 
        C[i] = 0;

    for(int i = 0; i < A_SIZE; i++) {
        for(int j = 0; j < B_SIZE; j++) {
            C[i + j] += A[i] * B[j];
        }
    }

    return C;
}

// After
void multiply_arrays2(int* A, int A_SIZE, int* B, int B_SIZE, int* C) {
    // Here calling FFT algorithm (check FFTTransform directory for examples and explanation)
}
```
