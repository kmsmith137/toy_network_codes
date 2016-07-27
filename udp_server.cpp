#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <iostream>

using namespace std;


int main(int argc, char **argv)
{
    static constexpr int expected_nbytes_per_packet = 4096;
    //    static constexpr int expected_npackets = 100000;
    static constexpr int udp_port = 13299;

    // FIXME is 2MB socket_bufsize a good choice?  I would have guessed a larger value 
    // would be better, but 2MB is the max allowed on my osx laptop.
    static constexpr int socket_bufsize = 2 << 21; 

    std::vector<uint8_t> packet(2 * expected_nbytes_per_packet, 0);

    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0)
	throw runtime_error(string("socket() failed: ") + strerror(errno));

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
	
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server_address.sin_addr);
    server_address.sin_port = htons(udp_port);
    
    int err = ::bind(sock_fd, (struct sockaddr *) &server_address, sizeof(server_address));
    if (err < 0)
	throw runtime_error(string("bind() failed: ") + strerror(errno));

    err = setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (void *) &socket_bufsize, sizeof(socket_bufsize));
    if (err < 0)
	throw runtime_error(string("setsockopt() failed: ") + strerror(errno));

    cout << "udp_server listening on port " << udp_port << "\n";

    for (;;) {
	int packet_nbytes = read(sock_fd, (char *) &packet[0], packet.size());
	if (packet_nbytes < 0)
	    throw runtime_error("read() failed");
	if (packet_nbytes != expected_nbytes_per_packet)
	    throw runtime_error("return value from read() is not equal to expected_nbytes_per_packet");
    }

    // FIXME show packet count
    return 0;
}
