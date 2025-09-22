#include <stdio.h>
#include <stdlib.h>
#include <time.h>


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

int main() {
    int N = 72000;
    int M = 34000;

    int *A = malloc(N * sizeof(int));
    int *B = malloc(M * sizeof(int));
    int *C = malloc((N + M - 1) * sizeof(int));

    srand(42);
    for(int i = 0; i < N; i++) 
        A[i] = rand() % 100 + 1;
    
    for(int i = 0; i < M; i++) 
        B[i] = rand() % 100 + 1;

    clock_t begin1 = clock();
    multiply_arrays1(A, N, B, M, C);
    clock_t end1 = clock();
    double time_spent1 = (double)(end1 - begin1) / CLOCKS_PER_SEC;   
    printf("First 5 elements: ");
    for(int i = 0; i < 5; i++) 
        printf("%d ", C[i]);
    printf("\tTime: %f s\n", time_spent1);

    clock_t begin2 = clock();
    int* C2 = multiply_arrays2(A, N, B, M);
    clock_t end2 = clock();
    double time_spent2 = (double)(end2 - begin2) / CLOCKS_PER_SEC;
    printf("First 5 elements: ");
    for(int i = 0; i < 5; i++) 
        printf("%d ", C2[i]);
    printf("\tTime: %f s\n", time_spent2);
    free(C2);

    free(A);
    free(B);
    free(C);

    return 0;
}
