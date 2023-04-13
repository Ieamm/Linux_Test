bin1=udpServer
src1=udpServer.cpp
bin2=udpClient
src2=udpClient.cpp
g=g++
P=.PHONY
lib=-lpthread
flag=-std=c++11

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