src=test.c
bin=test
g=gcc
# flag=-std=c++11
flag=-std=c99
P=.PHONY


$(bin) : $(src)
	$(g) -o $@ $^

$(P) : clean
clean:
	rm $(bin)
