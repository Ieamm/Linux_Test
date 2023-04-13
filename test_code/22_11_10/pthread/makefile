bin=mypthread
src=mypthread.cc
g=g++
flags=-std=c++11
P = .PHONY
pt_lib=pthread

$(bin) : $(src)
	$(g) -o $@ $^ -l$(pt_lib) -g $(flags)

$(P) : clean
clean:
	rm $(bin)