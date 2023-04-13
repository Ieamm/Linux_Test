bin = mypthread
src = mypthread.cc
flags = -std=c++11
lib = pthread
P = .PHONY
g = g++

$(bin) : $(src)
	$(g) -o $@ $^ -l$(lib) $(flags)

$(P) : clean
clean:
	rm $(bin)