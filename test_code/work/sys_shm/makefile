src1=server.cpp
bin1=server
src2=client.cpp
bin2=client
g=g++
flag=-std=c++11
P=.PHONY

$(P) : all
all:$(bin1) $(bin2)

$(bin1) : $(src1)
	$(g) -o $@ $^ $(flag)

$(bin2) : $(src2)
	$(g) -o $@ $^ $(flag)

$(P) : clean
clean:
	rm $(bin1)
	rm $(bin2)