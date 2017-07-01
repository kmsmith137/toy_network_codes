BINDIR=$(HOME)/bin
CPP=g++ -std=c++11 -Wall -O3 -pthread -I$(HOME)/include

BINFILES=dns-lookup local_socket_server udp_client udp_server tcp_client tcp_server
SCRIPTS=show-interrupts.py

BINFILES += zmq_pull_server zmq_push_client

all: $(BINFILES)

install: $(BINFILES)
	mkdir -p $(BINDIR)/ && cp -f $(BINFILES) $(SCRIPTS) $(BINDIR)/

clean:
	rm -f *~ $(BINFILES)

dns-lookup: dns-lookup.cpp
	$(CPP) -o $@ $<

local_socket_server: local_socket_server.cpp
	$(CPP) -o $@ $<

udp_client: udp_client.cpp lexical_cast.cpp lexical_cast.hpp time_inlines.hpp
	$(CPP) -o $@ udp_client.cpp lexical_cast.cpp

udp_server: udp_server.cpp argument_parser.cpp lexical_cast.cpp argument_parser.hpp lexical_cast.hpp time_inlines.hpp
	$(CPP) -o $@ udp_server.cpp argument_parser.cpp lexical_cast.cpp

tcp_client: tcp_client.cpp lexical_cast.cpp lexical_cast.hpp
	$(CPP) -o $@ tcp_client.cpp lexical_cast.cpp

tcp_server: tcp_server.cpp lexical_cast.cpp lexical_cast.hpp time_inlines.hpp
	$(CPP) -o $@ tcp_server.cpp lexical_cast.cpp

zmq_pull_server: zmq_pull_server.cpp
	$(CPP) -o $@ zmq_pull_server.cpp -lzmq

zmq_push_client: zmq_push_client.cpp
	$(CPP) -o $@ zmq_push_client.cpp -lzmq

