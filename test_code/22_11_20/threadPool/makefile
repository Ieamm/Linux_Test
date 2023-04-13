bin=threadPool_test
src=threadPool_test.cpp
lib=-lpthread
flag=-std=c++11
g=g++
P=.PHONY

$(bin) : $(src)
	$(g) -o $@ $^ $(lib) $(flag)

$(P) : clean
clean:
	rm $(bin)