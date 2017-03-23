CPP=g++ -std=c++11 -Wall -O3

all: dns-lookup local_socket_server udp_client udp_server

dns-lookup: dns-lookup.cpp
	$(CPP) -o $@ $<

local_socket_server: local_socket_server.cpp
	$(CPP) -o $@ $<

udp_client: udp_client.cpp lexical_cast.cpp lexical_cast.hpp time_inlines.hpp
	$(CPP) -o $@ udp_client.cpp lexical_cast.cpp

udp_server: udp_server.cpp lexical_cast.cpp lexical_cast.hpp time_inlines.hpp
	$(CPP) -o $@ udp_server.cpp lexical_cast.cpp

tcp_client: tcp_client.cpp lexical_cast.cpp lexical_cast.hpp time_inlines.hpp
	$(CPP) -o $@ tcp_client.cpp lexical_cast.cpp
