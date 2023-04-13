bin=Server
src=Server.cc

$(bin) : $(src)
	g++ -o $@ $^ -std=c++11

.PHONY : clean
clean:
	rm -f $(bin)
