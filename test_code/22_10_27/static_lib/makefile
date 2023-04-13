mytest.exe : mytest.c
	gcc $^ -o $@ -I ./lib-static/include/ -L ./lib-static/lib/ -l mymath

.PHONY : clean
clean:
	rm -f mytest.exe
