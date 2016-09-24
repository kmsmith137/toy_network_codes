CPP=g++ -std=c++11 -Wall -O3

all: local_socket_server udp_client udp_server

local_socket_server: local_socket_server.cpp
	$(CPP) -o $@ $<

udp_client: udp_client.cpp
	$(CPP) -o $@ $<

udp_server: udp_server.cpp
	$(CPP) -o $@ $<
