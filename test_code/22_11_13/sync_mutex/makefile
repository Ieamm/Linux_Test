bin = mythread
src = mythread.cc
g = g++
flags = -std=c++11
lib = pthread
P = .PHONY

$(bin) : $(src)
	$(g) -o $@ $^ -l$(lib) $(flags)

$(P) : clean
clean:
	rm $(bin)