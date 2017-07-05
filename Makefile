

all:
	g++ -std=c++11 scottclient.cpp -I/usr/include/libindi/ -lindiclient -lz -lnova -lpthread -o scottclient -luWS -lssl
