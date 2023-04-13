bin1=tcpServer
src1=tcpServer.cpp
bin2=tcpClient
src2=tcpClient.cpp
flag=-std=c++11
lib=-lpthread
g=g++
p=.PHONY

$(P) : all
all : $(bin1) $(bin2)

$(bin1) : $(src1)
	$(g) -o $@ $^ $(lib) $(flag)

$(bin2) : $(src2)
	$(g) -o $@ $^ $(lib) $(flag)

$(P) : clean
clean:
	rm $(bin1)
	rm $(bin2)