#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846


typedef struct {
    double real;
    double imag;
} Complex;


Complex complex_add(Complex a, Complex b) {
    Complex res = {
        a.real + b.real, 
        a.imag + b.imag
    };

    return res;
}


Complex complex_sub(Complex a, Complex b) {
    Complex res = {
        a.real - b.real, 
        a.imag - b.imag
    };

    return res;
}


Complex complex_mul(Complex a, Complex b) {
    Complex res = {
        (a.real * b.real) - (a.imag * b.imag), 
        (a.real * b.imag) + (a.imag * b.real)
    };

    return res;
}


void fft(Complex *a, int n, int invert) {
    for(int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;

        for(; j & bit; bit >>= 1)
            j ^= bit;
        
        j |= bit;
        if(i < j) {
            Complex temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
    }

    for(int len = 2; len <= n; len <<= 1) {
        double angle = 2 * PI / len * (invert ? -1 : 1);
        Complex wlen = { 
            cos(angle), 
            sin(angle) 
        };

        for(int i = 0; i < n; i += len) {
            Complex w = { 1, 0 };
            for(int j = 0; j < len/2; j++) {
                Complex u = a[i + j];
                Complex v = complex_mul(a[i + j + len/2], w);
                a[i + j] = complex_add(u,v);
                a[i + j + len/2] = complex_sub(u,v);
                w = complex_mul(w, wlen);
            }
        }
    }

    if(invert) {
        for(int i = 0; i < n; i++) {
            a[i].real/=n;
            a[i].imag/=n;
        }
    }
}


void fft_polynomial_mul_void(int *A, int A_SIZE, int *B, int B_SIZE, int *C) {
    int size = 1;
    while(size < A_SIZE + B_SIZE) {
        size <<= 1;
    }

    Complex *fa = (Complex*) calloc(size, sizeof(Complex));
    Complex *fb = (Complex*) calloc(size, sizeof(Complex));

    for(int i = 0; i < A_SIZE; i++) {
        fa[i].real = A[i];
    }

    for(int i = 0; i < B_SIZE; i++) {
        fb[i].real = B[i];
    }

    fft(fa,size,0);
    fft(fb,size,0);

    for(int i = 0; i < size; i++) {
        fa[i] = complex_mul(fa[i], fb[i]);
    }

    fft(fa,size,1);

    for(int i = 0; i < A_SIZE + B_SIZE - 1; i++) {
        C[i] = (int)(fa[i].real + 0.5);
    }

    free(fa);
    free(fb);
}


int* fft_polynomial_mul(int *A, int A_SIZE, int *B, int B_SIZE) {
    int *C = (int*) malloc((A_SIZE + B_SIZE - 1) * sizeof(int));
    fft_polynomial_mul_void(A, A_SIZE, B, B_SIZE, C);

    return C;
}
