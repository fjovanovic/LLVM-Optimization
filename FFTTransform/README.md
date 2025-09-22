## Folder structure:
  - build/fft_polynomial.mul.c
  - build/test.c
  - llvm/lib/Transforms/FFTTransform/FFTTransform.cpp
  - llvm/lib/Transforms/CMakeLists.txt add FFT pass:  
    ```
    add_subdirectory(Utils)
    ...
    add_subdirectory(FFTTransform)
    ```
  - llvm/lib/Transformers/FFTTransform/CMakeLists.txt:  
    ```
    add_llvm_library(LLVMFFTTransform MODULE
      FFTTransform.cpp

      PLUGIN_TOOL
      opt
    )
    ``` 


## Build LLVM from source
  - llvm-project/./make_llvm.sh
  - cd build
  - ./make_fft.sh