CPP=g++ -std=c++11 -Wall -O3

all: udp_client

udp_client: udp_client.cpp
	$(CPP) -o $@ $<
