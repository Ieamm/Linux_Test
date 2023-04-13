.PHONY : all
all : read write

read : read.cc
	g++ -o $@ $^ -std=c++11

write : write.cc 
	g++ -o $@ $^ -std=c++11

.PHONY : clean
clean:
	rm -f read 
	rm -f write 
