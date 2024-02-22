#
#	
#
#	referenced stack overflow post here
#		https://stackoverflow.com/questions/14639794/getting-make-to-create-object-files-in-a-specific-directory
#
#
INCLUDE_DIRS	= 
LIB_DIRS 		= 
CC 				= gcc
CDEFS 			=
CFLAGS 			= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS 			= -pthread

HFILES 	= 
CFILES 	= src/feasibility_tests.c
SRCS 	= ${HFILES} ${CFILES}
OBJS 	= bin/feasibility_tests.o
TRGT	= bin/feasibility_tests

all: build feasibility_tests

build:
	mkdir -p bin

feasibility_tests: feasibility_tests.o
	$(CC) $(LIBS) $(CFLAGS) $(OBJS) -o $(TRGT) -lm

feasibility_tests.o: build
	$(CC) $(LIBS) $(CFLAGS) -c $(SRCS) -o $(OBJS) -lm

clean:
	rm -r bin