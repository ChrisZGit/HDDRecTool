
.PHONY: pathmaker clean

CC=g++
OPT=-g -m64 -std=c++11 -O2 -Wall -Wshadow -Wextra -pedantic
LIBS=-pthread
INCPATH=./include/
SRCPATH=./src/
INC=-I$(INCPATH) -I$(SRCPATH)
MAIN=$(SRCPATH)main.cpp

BINPATH=./bin/
OBJPATH=./obj/

objects= \
		 $(OBJPATH)edbHandler.o\
		 $(OBJPATH)imageCarver.o\
		 $(OBJPATH)hashSumHandler.o\
		 $(OBJPATH)dbHandler.o\
		 $(OBJPATH)dataHandler.o\
		$(OBJPATH)fileReader.o\
		$(OBJPATH)fileWriter.o\


all: exe

pathmaker:
	mkdir -p obj/ bin/

exe: pathmaker $(objects) $(MAIN)
	$(CC) $(OPT) $(INC) $(LIBS) -o $(BINPATH)runme $(objects) $(MAIN)

$(OBJPATH)%.o: $(SRCPATH)%.cpp $(INCPATH)%.h
	$(CC) $(OPT) $(INC) $(LIBS) -c $< -o $@

clean:
	rm -rf $(OBJPATH) $(BINPATH)/runme
