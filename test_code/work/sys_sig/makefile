src=test.cpp
bin=test
g=g++
flag=-std=c++11
P=.PHONY


$(bin) : $(src)
	$(g) -o $@ $^ $(flag)

$(P) : clean
clean:
	rm $(bin)
