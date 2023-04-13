bin=rwlock_test
src=rwlock_test.cpp
lib=-lpthread
flag=-std=c++11

$(bin) : $(src)
	g++ -o $@ $^ $(lib) $(flag)

.PHONY : clean
clean:
	rm $(bin)