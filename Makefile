CXX = g++
CXXFLAGS = -g -Wall -Wextra -Wno-sign-compare -O3

HEADERS = src/backlinks_compression.h
OBJECTS = $(subst .h,.o,$(HEADERS))

LIB = -lgtest -lgtest_main -lpthread
GTEST_ROOT= lib/gtest-1.6.0
JLOG_ROOT = lib/cpp-json-logger

all: bin bin/test

bin:
	mkdir -p bin
	
.h.o:
	$(CXX) -c $< 

bin/test: src/backlinks_compression_test.cc src/backlinks_compression.cc ${HEADERS} 
	$(CXX) $(CXXFLAGS) -o $@ $^ -I $(GTEST_ROOT)/include -L \
	$(GTEST_ROOT)/lib $(LIB)
	./$@

.PHONY:	test clean

clean:
	rm -r bin
	rm src/*.o
