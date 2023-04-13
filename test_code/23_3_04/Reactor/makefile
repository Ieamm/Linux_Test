.PHONY : all
all : Client Server

Client : TcpClient.cc
	g++ -o $@ $^ -ljsoncpp -std=c++11

Server : TcpServer.cc
	g++ -o $@ $^ -ljsoncpp -std=c++11

.PHONY : clean
clean:
	rm -f Server Client