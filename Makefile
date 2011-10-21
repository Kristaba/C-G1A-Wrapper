

all: c_g1awrapper

c_g1awrapper: bmp_utils.o c_g1awrapper.o
	gcc -o c_g1awrapper bmp_utils.o c_g1awrapper.o

c_g1awrapper.o: c_g1awrapper.c bmp_utils.h g1a_header.h
	gcc -c -o c_g1awrapper.o c_g1awrapper.c

bmp_utils.o: bmp_utils.c bmp_utils.h
	gcc -c -o bmp_utils.o bmp_utils.c

clean:
	rm bmp_utils.o c_g1awrapper.o

realclean: clean
	rm c_g1awrapper

install: all
	cp c_g1awrapper /usr/bin
