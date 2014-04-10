
.PHONY: pathmaker clean

CC=g++
OPT=-g -O2 -std=c++11 -Wall -Wshadow -Wextra -pedantic
LIBS=-pthread
INCPATH=./include/
SRCPATH=./src/
INC=-I$(INCPATH) -I$(SRCPATH)
MAIN=$(SRCPATH)main.cpp

BINPATH=./bin/
OBJPATH=./obj/

objects=

all: exe

pathmaker:
	mkdir -p obj/ bin/

exe: pathmaker $(objects) $(MAIN)
	$(CC) $(OPT) $(INC) $(LIBS) -o $(BINPATH)runme $(objects) $(MAIN)

clean:
	rm -rf $(OBJPATH) $(BINPATH)
