add_llvm_library(LLVMRemoveUnusedFunctionArguments MODULE
  RemoveUnusedFunctionArguments.cpp

  PLUGIN_TOOL
  opt
)