CC = g++
CFLAGS = -g -Wall -O0 -std=c++23 -mavx2 -march=native -fopenmp -I.

.DEFAULT_GOAL := after_optimize
.PHONY: clean

all: before_optimize after_optimize

before_optimize: before_optimize.cpp
	$(CC) $(CFLAGS) -o $@ $^

after_optimize: after_optimize.cpp
	$(CC) $(CFLAGS) -o $@ $^
	
clean:
	rm -f before_optimize after_optimize