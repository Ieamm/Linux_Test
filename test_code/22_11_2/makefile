.PHONY : all
all : ipcShmCli ipcShmSer

ipcShmSer : IpcShmSer.cc
	g++ -o $@ $^ -std=c++11
	
ipcShmCli : IpcShmCli.cc
	g++ -o $@ $^ -std=c++11

.PHONY : clean
clean:
	rm -f ipcShmCli
	rm -f ipcShmSer
	