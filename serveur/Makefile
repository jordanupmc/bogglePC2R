#				PC2R
#		   Makefile pour le serveur Boggle

CC = gcc
DIR=.
BIN=$(DIR)/bin/
DIROBJ= $(DIR)/obj/
OBJ =  $(DIROBJ)serveur.o  $(DIROBJ)grille.o $(DIROBJ)game.o $(DIROBJ)threadSafeList.o
SRC=$(DIR)/src/
LDFLAGS = -pthread -Wall -O3 

.SUFFIXES:
.PHONY: all clean prog 

all: $(BIN)server


$(BIN)server: $(OBJ)
	@if [ -d $(BIN) ]; then : ; else mkdir $(BIN); fi
	$(CC) $(LDFLAGS) -o $@ $(OBJ) -lm

$(DIROBJ)serveur.o: $(SRC)serveur.c $(SRC)serveur.h
	@if [ -d $(DIROBJ) ]; then : ; else mkdir $(DIROBJ); fi
	$(CC) -c $(LDFLAGS) -o $@ $< -lm

$(DIROBJ)grille.o: $(SRC)grille.c $(SRC)grille.h
	@if [ -d $(DIROBJ) ]; then : ; else mkdir $(DIROBJ); fi
	$(CC) -c  $(LDFLAGS) -o $@  $< -lm

$(DIROBJ)game.o: $(SRC)game.c $(SRC)game.h
	@if [ -d $(DIROBJ) ]; then : ; else mkdir $(DIROBJ); fi
	$(CC) -c  $(LDFLAGS) -o $@  $< -lm

$(DIROBJ)threadSafeList.o: $(SRC)threadSafeList.c $(SRC)threadSafeList.h
	@if [ -d $(DIROBJ) ]; then : ; else mkdir $(DIROBJ); fi
	$(CC) -c  $(LDFLAGS) -o $@  $< -lm
clean:
	rm -rf $(BIN) $(DIROBJ)
