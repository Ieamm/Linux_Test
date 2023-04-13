src=test.cpp
bin=test
g=g++
flag=-std=c++11
P=.PHONY
lib=pthread

$(bin) : $(src)
	$(g) -o $@ $^ -l$(lib) $(flag)

$(P) : clean
clean:
	rm $(bin)
