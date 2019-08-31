CC = gcc
CFLAGS = -Wall -pedantic
CLIBS = -lcmark

default: dirs main.c
	$(CC) $(CFLAGS) main.c -o ./bin/cld-ssg $(CLIBS)

dirs:
	if [ ! -d ./bin ] ; then mkdir bin ; fi

clean:
	if [ -d ./bin ] ; then rm -rf ./bin/* ; fi


