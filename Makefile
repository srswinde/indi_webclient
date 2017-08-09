src=src
include=$(src)
CC=g++
CFLAGS=-std=c++11 -Wall
LIBS=-lindiclient -lz -lnova -lpthread -luWS -lssl
EXE=webclient

all:
	$(CC) $(CFLAGS) $(src)/webclient.cpp -I/usr/include/libindi/ -o $(EXE) $(LIBS)



