#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <cstring>
#include <iostream>
#include <stdexcept>

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


int main(int argc, char **argv)
{
    int sockfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sockfd < 0)
	throw runtime_error(string("socket() failed: ") + strerror(errno));

#if 0
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
	
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server_address.sin_addr);
    server_address.sin_port = htons(udp_port);
    
    int err = ::bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (err < 0)
	throw runtime_error(string("bind() failed: ") + strerror(errno));

    err = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void *) &socket_bufsize, sizeof(socket_bufsize));
    if (err < 0)
	throw runtime_error(string("setsockopt() failed: ") + strerror(errno));

    cout << "udp_server listening on port " << udp_port << "\n"
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
#endif
    return 0;
}
