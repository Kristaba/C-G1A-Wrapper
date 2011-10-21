CC=gcc
CFLAGS=-W -Wall
LDFLAGS=

all: c_g1awrapper


c_g1awrapper: bmp_utils.o c_g1awrapper.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

c_g1awrapper.o: bmp_utils.h g1a_header.h

bmp_utils.o: bmp_utils.h


.PHONY: clean mrproper realclean

clean:
	rm bmp_utils.o c_g1awrapper.o

mrproper: clean
	rm c_g1awrapper

realclean: mrproper


install: all
	cp c_g1awrapper /usr/bin
