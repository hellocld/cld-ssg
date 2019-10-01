CC = gcc
CFLAGS = -g -Wall -pedantic 
CLIBS = -lcmark

INSTALLDIR = /usr/local/bin/

default: dirs main.c config.h util
	$(CC) $(CFLAGS) main.c ./bin/util.o -o ./bin/cld-ssg $(CLIBS)

dirs:
	if [ ! -d ./bin ] ; then mkdir bin ; fi

util: util.h util.c
	$(CC) $(CFLAGS) util.c -c -o ./bin/util.o

clean:
	if [ -d ./bin ] ; then rm -rf ./bin/* ; fi

install: clean default
	cp -f ./bin/cld-ssg $(INSTALLDIR)

uninstall:
	rm -f $(INSTALLDIR)cld-ssg

.PHONY: clean install uninstall
