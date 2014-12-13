# libsvm_threads

Parallel port of [LIBSVM][1] using C++11 threads.  This port requires a compliant C++11 compiler (e.g. g++ 4.7 or 4.8).  

## Installation

Modify the CXX macro in the Makefile to refer to a C++11 compiler. Then,

    make clean
	make
	./svm-train_par -h

[1]: http://www.csie.ntu.edu.tw/~cjlin/libsvm/


