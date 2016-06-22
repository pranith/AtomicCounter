add_library(LLVMAtomicCount MODULE
    AtomicCount.cpp
)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(LLVMAtomicCount PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that.
set_target_properties(LLVMAtomicCount PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
