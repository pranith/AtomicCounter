Atomic Counter
---

An LLVM pass to count the number of dynamic atomic instructions executed.

Build
---

	$ mkdir build
	$ cd build
	$ LLVM_DIR=<path to llvm build> cmake ..

Compile
---
	$ <llvm build>/bin/clang -Xclang -load -Xclang AtomicCounter/build/AtomicCountPass/libLLVMAtomicCount.so test.c test1.c -o test

Run
---
	$ ./test
	Atomic Counter: xx
