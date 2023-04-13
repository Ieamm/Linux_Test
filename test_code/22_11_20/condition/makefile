bin=BlockQueue
src=BlockQueue.cc
lib=-lpthread
flag=-std=c++11
g=g++
P=.PHONY

$(bin) : $(src)
	$(g) -o $@ $^ $(lib) $(flag)


$(P) : clean
clean:
	rm con-pro