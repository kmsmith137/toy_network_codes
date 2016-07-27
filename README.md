# toy_network_codes

 - udp_client.cpp: a simple udp client which tries to match its packet rate to a specified target (in Gbps),
     using the socket API and a loop containing gettimeofday(), usleep(), send().

 - udp_server.cpp: simple server which uses socket API and read() loop.
