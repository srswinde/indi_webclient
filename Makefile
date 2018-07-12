src=src
include=$(src)
CC=g++
CFLAGS=-std=c++11 -Wall
LIBS=-lindiclient -lz -lnova -lpthread -lssl
EXE=webclient

all: clean webclient

webclient:
	$(CC) $(CFLAGS) $(src)/webclient.cpp -I/usr/include/libindi/ -I json/include -o $(EXE) $(LIBS)



install: webclient
	cp webclient /usr/local/bin/webclient-indi
	cp systemd/webclient-indi.service /etc/systemd/system
	chmod 664 /etc/systemd/system/webclient-indi.service
	systemctl daemon-reload
	systemctl enable webclient-indi.service
	

clean: 
	rm -rf webclient
