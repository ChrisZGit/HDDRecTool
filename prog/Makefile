
.PHONY: pathmaker clean

CC=g++
OPT=-g -O2 -Wall -Wshadow -Wextra -pedantic
LIBS=-pthread
INCPATH=./include/
SRCPATH=./src/
INC=-I$(INCPATH) -I$(SRCPATH)
MAIN=$(SRCPATH)main.cpp

BINPATH=./bin/
OBJPATH=./obj/

objects=$(OBJPATH)raidSystem.o\
		$(OBJPATH)fileHandler.o\
		$(OBJPATH)fileReader.o


all: exe

pathmaker:
	mkdir -p obj/ bin/

exe: pathmaker $(objects) $(MAIN)
	$(CC) $(OPT) $(INC) $(LIBS) -o $(BINPATH)runme $(objects) $(MAIN)

$(OBJPATH)raidSystem.o: $(INCPATH)raidSystem.h $(SRCPATH)raidSystem.cpp
	$(CC) $(OPT) $(INC) $(LIBS) -c -o $(OBJPATH)raidSystem.o $(SRCPATH)raidSystem.cpp

$(OBJPATH)fileReader.o: $(INCPATH)fileReader.h $(SRCPATH)fileReader.cpp
	$(CC) $(OPT) $(INC) $(LIBS) -c -o $(OBJPATH)fileReader.o $(SRCPATH)fileReader.cpp

$(OBJPATH)fileHandler.o: $(INCPATH)fileHandler.h $(SRCPATH)fileHandler.cpp
	$(CC) $(OPT) $(INC) $(LIBS) -c -o $(OBJPATH)fileHandler.o $(SRCPATH)fileHandler.cpp

clean:
	rm -rf $(OBJPATH) $(BINPATH)
