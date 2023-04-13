bin = mypthread
src = mypthread.cc
g = g++
flags = -std=c++11
P = .PHONY
lib = pthread

$(bin) : $(src)
	$(g) -o $@ $^ -l$(lib) $(flags)


$(P) : clean
clean:
	rm $(bin)
