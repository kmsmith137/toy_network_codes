#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <cstring>
#include <iostream>

#include "time_inlines.hpp"
#include "lexical_cast.hpp"

using namespace std;


static void usage()
{
    cerr << "usage: udp_server <ip_addr> <port>\n";
    exit(2);
}


int main(int argc, char **argv)
{
#ifdef __APPLE__
    // osx seems to have very small limits on socket buffer size
    static constexpr int socket_bufsize = 4 * 1024 * 1024;
#else
    static constexpr int socket_bufsize = 128 * 1024 * 1024;
#endif

    // too large, but that's ok!
    static constexpr int max_packet_size = 65536;

    if (argc != 3)
	usage();

    string ip_addr = argv[1];
    int udp_port = lexical_cast<int> (argv[2], "udp_port");

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
	throw runtime_error(string("socket() failed: ") + strerror(errno));

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(udp_port);

    int err = inet_pton(AF_INET, ip_addr.c_str(), &saddr.sin_addr);
    if (err <= 0)
	throw runtime_error(ip_addr + ": inet_pton() failed (note that no DNS lookup is done, the argument must be a numerical IP address)");

    err = ::bind(sockfd, (struct sockaddr *) &saddr, sizeof(saddr));
    if (err < 0)
	throw runtime_error(string("bind() failed: ") + strerror(errno));
    
    struct timeval tv_timeout = { 1, 0 };   // 1 second
    
    err = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv_timeout, sizeof(struct timeval));
    if (err < 0)
	throw runtime_error(string("setsockopt() failed: ") + strerror(errno));

    err = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void *) &socket_bufsize, sizeof(socket_bufsize));
    if (err < 0)
	throw runtime_error(string("setsockopt() failed: ") + strerror(errno));

    cout << "udp_server listening on " << ip_addr << ":" << udp_port << "\n"
	 << "The udp_server will delimit \"streams\" based on 1-second intervals with no packets, and report timings for each stream.\n";

    std::vector<uint8_t> packet(max_packet_size+1, 0);

    // Outer loop over streams.
    for (;;) {
	struct timeval tv_start = xgettimeofday();
	struct timeval tv_end = xgettimeofday();
	ssize_t npackets_received = 0;
	ssize_t nbytes_received = 0;

	// Inner loop over packets.
	for (;;) {
	    int packet_nbytes = read(sockfd, (char *) &packet[0], packet.size());
	
	    if (packet_nbytes < 0) {
		if ((errno != EAGAIN) && (errno != ETIMEDOUT))
		    throw runtime_error(string("read() failed: ") + strerror(errno));
		if (npackets_received > 0)
		    break;   // timed out, end of stream
		continue;    // timed out, but still waiting for first packet
	    }

	    if (packet_nbytes > max_packet_size)
		throw runtime_error("packet exceeded max allowed size?!");

	    if (npackets_received == 0) {
		cout << "received first packet in stream\n";
		tv_start = xgettimeofday();
	    }

	    tv_end = xgettimeofday();
	    nbytes_received += packet_nbytes;
	    npackets_received++;
	}

	double secs_elapsed = secs_between(tv_start, tv_end);
	double gbps = 8.0e-9 * nbytes_received / secs_elapsed;
	cout << "udp_server: received " << npackets_received << " packets in " << secs_elapsed << " secs (" << gbps << " gpbs)\n";
    }

    return 0;
}
