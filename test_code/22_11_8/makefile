bin=mysignal
src=mysignal.cc
g=g++
FLAGS=-std=c++11

$(bin) : $(src)
	$(g) -o $@ $^ -O3 $(FLAGS)

.PHONY : clean
clean:
	rm $(bin)