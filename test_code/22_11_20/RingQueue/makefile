bin=RingQueue_test
src=RingQueue_test.cpp
flag=-std=c++11
lib=-lpthread
g=g++
P=.PHONY

$(bin) : $(src)
	$(g) -o $@ $^ $(lib) $(flag)

$(P) : clean
clean:
	rm $(bin)