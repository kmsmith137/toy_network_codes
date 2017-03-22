#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "lexical_cast.hpp"

using namespace std;


inline struct timeval xgettimeofday()
{
    struct timeval tv;

    int err = gettimeofday(&tv, NULL);
    if (err)
	throw runtime_error("gettimeofday failed");

    return tv;
}

inline double secs_between(struct timeval &tv1, struct timeval &tv2)
{
    return (tv2.tv_sec - tv1.tv_sec) + 1.0e-6*(tv2.tv_usec - tv1.tv_usec);
}


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

    err = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void *) &socket_bufsize, sizeof(socket_bufsize));
    if (err < 0)
	throw runtime_error(string("setsockopt() failed: ") + strerror(errno));

    cout << "udp_server listening on " << ip_addr << ":" << udp_port << "\n"
	 << "A timer will start when the first packet is received.\n"
	 << "Subsequently, if no packets are received in a 1-sec interval, the timer will stop and the server will exit.\n";

    std::vector<uint8_t> packet(max_packet_size+1, 0);
    struct timeval tv_start = xgettimeofday();
    struct timeval tv_end = xgettimeofday();
    ssize_t npackets_received = 0;
    ssize_t nbytes_received = 0;

    for (;;) {
	int packet_nbytes = read(sockfd, (char *) &packet[0], packet.size());
	
	if (packet_nbytes < 0) {
	    if ((errno == EAGAIN) || (errno == ETIMEDOUT))
		break;   // read() timed out
	    throw runtime_error(string("read() failed: ") + strerror(errno));
	}

	if (packet_nbytes > max_packet_size)
	    throw runtime_error("packet exceeded max allowed size?!");

	tv_end = xgettimeofday();
	nbytes_received += packet_nbytes;
	npackets_received++;

	if (npackets_received > 1)
	    continue;

	// If we get here, this is the first packet!  Update tv_start to its arrival time.
	tv_start = tv_end;

	// Set 1-sec timeout for subsequent calls to read().
	struct timeval tv_timeout = { 1, 0 };

	err = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv_timeout, sizeof(struct timeval));
	if (err < 0)
	    throw runtime_error(string("setsockopt() failed: ") + strerror(errno));
    }

    double secs_elapsed = secs_between(tv_start, tv_end);
    double gbps = 8.0e-9 * nbytes_received / secs_elapsed;

    if ((errno != EAGAIN) && (errno != EWOULDBLOCK) && (errno != ETIMEDOUT))
	throw runtime_error("read() failed");
    if (npackets_received < 2)
	throw runtime_error("udp_server received < 2 packets");

    cout << "udp_server: received " << npackets_received << " packets in " << secs_elapsed << " secs (" << gbps << " gpbs)\n";
    return 0;
}
