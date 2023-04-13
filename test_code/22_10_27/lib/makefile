.PHONY : all
all : libmymath.so libmymath.a

libmymath.so: mymath.o myprint.o
	gcc -shared -o libmymath.so mymath.o myprint.o
mymath.o : mymath.c
	gcc -fPIC -c mymath.c -o mymath.o -std=c99
myprint.o : myprint.c
	gcc -fPIC -c myprint.c -o myprint.o -std=c99


libmymath.a : mymath_s.o myprint_s.o
	ar -rc libmymath.a mymath_s.o myprint_s.o
mymath_s.o : mymath.c
	gcc -c mymath.c -o mymath_s.o -std=c99
myprint_s.o : myprint.c
	gcc -c myprint.c -o myprint_s.o -std=c99

.PHONY : lib 
lib:
	mkdir -p lib-static/lib 
	mkdir -p lib-static/include
	cp *.a lib-static/lib 
	cp *.h lib-static/include
	mkdir -p lib-dyl/lib 
	mkdir -p lib-dyl/include
	cp *.so lib-dyl/lib 
	cp *.h lib-dyl/include

.PHONY : clean 
clean:
	rm -f *.a
	rm -f *.so 
	rm -f *.o
	rm -rf lib-static 
	rm -rf lib-dyl 




