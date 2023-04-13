src=BuyTickets.cpp
bin=BuyTickets
g=g++
flag=-std=c++11
P=.PHONY
lib=pthread

$(bin) : $(src)
	$(g) -o $@ $^ -l$(lib) $(flag)

$(P) : clean
clean:
	rm $(bin)
