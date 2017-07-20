


all:
	g++ -std=c++11 scottclient.cpp -I/usr/include/libindi/ -lindiclient -lz -lnova -lpthread -o scottclient -luWS -lssl

sdev:
	g++ -std=c++11 scottdevice.cpp -I/usr/include/libindi -lindidriver -lnova -lpthread -lz -o sdev


