bin1=tcpServer
src1=tcpServer.cpp
bin2=tcpClient
src2=tcpClient.cpp
flag=-std=c++11
lib1=-lpthread
lib2=-ljsoncpp
Json=#-DJSON

.PHONY : all
all : $(bin1) $(bin2)

$(bin1) : $(src1)
	g++ -o $@ $^ $(lib1) $(lib2) $(Json) $(flag)

$(bin2) : $(src2)
	g++ -o $@ $^ $(lib2) $(Json) $(flag)

.PHONY : clean
clean:
	rm $(bin1)
	rm $(bin2)