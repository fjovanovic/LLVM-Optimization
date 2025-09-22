#!/bin/bash
set -e

rm -f fft_polynomial_mul.o test.bc test_opt.bc test_opt test_noopt test_noopt.ll test_opt.ll

gcc -c fft_polynomial_mul.c -o fft_polynomial_mul.o

./bin/clang -O0 -emit-llvm -c test.c -o test.bc
./bin/opt -load ./lib/LLVMFFTTransform.so -enable-new-pm=0 -fft-pass < test.bc > test_opt.bc

./bin/clang test_opt.bc fft_polynomial_mul.o -o test_opt -lm
./bin/clang test.bc fft_polynomial_mul.o -o test_noopt -lm

echo -e "\nRunning non-optimized code:"
time ./test_noopt

echo -e "\nRuning optimized code"
time ./test_opt

./bin/llvm-dis test.bc -o test_noopt.ll
./bin/llvm-dis test_opt.bc -o test_opt.ll
