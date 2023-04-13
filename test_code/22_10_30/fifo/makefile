.PHONY : all
all : clientfifo.o serverfifo.o

clientfifo.o : clientfifo.cpp
	g++ $^ -o $@ -std=c++11 -g

serverfifo.o : serverfifo.cpp
	g++ $^ -o $@ -std=c++11 -g

.PHONY : clean
clean:
	rm *.o
	rm fifo