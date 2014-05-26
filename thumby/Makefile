
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
		 $(OBJPATH)thumbWriter.o\
		 $(OBJPATH)fileWriter.o\
		 $(OBJPATH)texMaker.o\
		 $(OBJPATH)xmlMaker.o\
		 $(OBJPATH)imageCarver.o\
		 $(OBJPATH)hashSumHandler.o\
		 $(OBJPATH)dbHandler.o\
		 $(OBJPATH)dataHandler.o\

all: pathmaker
all: thumb
all: exe

pathmaker:
	mkdir -p obj/ bin/

thumb: ./thumbviewer/main.cpp
	$(CC) $(OPT) thumbviewer/main.cpp -o bin/thumbCV && echo '' && echo '' && echo "Real thumby compilation now" && echo ''

exe: $(objects) $(MAIN)
	$(CC) $(OPT) $(INC) $(LIBS) -o $(BINPATH)runme $(objects) $(MAIN)

$(OBJPATH)%.o: $(SRCPATH)%.cpp $(INCPATH)%.h
	$(CC) $(OPT) $(INC) $(LIBS) -c $< -o $@

clean:
	rm -rf $(OBJPATH) $(BINPATH)
