CXX = g++
CFLAGS = -Wall -Wconversion -O3 -std=c++11
SHVER = 2
OS = $(shell uname)

all: svm-train_par svm-train svm-predict svm-scale

lib: svm.o
	if [ "$(OS)" = "Darwin" ]; then \
		SHARED_LIB_FLAG="-dynamiclib -Wl,-install_name,libsvm.so.$(SHVER)"; \
	else \
		SHARED_LIB_FLAG="-shared -Wl,-soname,libsvm.so.$(SHVER)"; \
	fi; \
	$(CXX) $${SHARED_LIB_FLAG} svm.o -o libsvm.so.$(SHVER)

svm-predict: svm-predict.c svm.o
	$(CXX) $(CFLAGS) svm-predict.c svm.o -o svm-predict -lm
svm-train: svm-train.c svm.o
	$(CXX) $(CFLAGS) svm-train.c svm.o -o svm-train -lm
svm-train_par: svm-train.c svm_par.o
	$(CXX) $(CFLAGS) svm-train.c svm_par.o -o svm-train_par -lm
svm-scale: svm-scale.c
	$(CXX) $(CFLAGS) svm-scale.c -o svm-scale
svm.o: svm.cpp svm.h
	$(CXX) $(CFLAGS) -c svm.cpp
svm_par.o: svm_par.cpp svm.h ThreadPool/SvmThreads.h ThreadPool/ThreadPool.h
	$(CXX) $(CFLAGS) -c svm_par.cpp
clean:
	rm -f *~ svm.o svm_par.o svm-train svm-predict svm-scale libsvm.so.$(SHVER)
