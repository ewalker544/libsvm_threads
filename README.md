# libsvm_threads

Parallel port of [LIBSVM][1] using C++11 threads.  This port requires a compliant C++11 compiler.  Tested with 

	gcc 4.7.1, 4.8.3
	icpc 13.0.1.117, 13.1.1.163, 14.0.1.106

## Installation

Modify the CXX macro in the Makefile to refer to a C++11 compiler. Then,

    make clean
	make
	./svm-train-par -h

[1]: http://www.csie.ntu.edu.tw/~cjlin/libsvm/


