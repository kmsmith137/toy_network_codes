#include <unistd.h>
#include <arpa/inet.h>

#include <vector>
#include <cstring>
#include <cassert>
#include <iostream>
#include <stdexcept>

#include "lexical_cast.hpp"

using namespace std;


static void usage()
{
    cerr << "usage: tcp_client <ip_addr> <port>\n";
    exit(2);
}


int main(int argc, char **argv)
{
    // Write data in 1MB chunks
    static constexpr int chunk_size = 1024 * 1024;

    if (argc != 3)
	usage();

    string ip_addr = argv[1];
    int tcp_port = lexical_cast<int> (argv[2], "tcp_port");

    assert(tcp_port > 0 && tcp_port < 65536);

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(tcp_port);

    int err = inet_pton(AF_INET, ip_addr.c_str(), &saddr.sin_addr);
    if (err <= 0)
	throw runtime_error(ip_addr + ": inet_pton() failed (note that no DNS lookup is done, the argument must be a numerical IP address)");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        throw runtime_error(string("socket() failed: ") + strerror(errno));

    // Note: bind() not called, so source port number of outgoing packets will be arbitrarily assigned

    if (connect(sockfd, reinterpret_cast<struct sockaddr *> (&saddr), sizeof(saddr)) < 0) {
        close(sockfd);
        throw runtime_error(string("connect() failed: ") + strerror(errno));
    }

    vector<char> chunk(chunk_size, 0);

    cout << "tcp_client: writing to " << ip_addr << ":" << tcp_port << "\n"
	 << "Press control-C to exit!" << endl;

    for (;;) {
	ssize_t n = send(sockfd, &chunk[0], chunk_size, 0);
	if (n < 0)
	    throw runtime_error(string("send() failed: ") + strerror(errno));
	if (n != chunk_size)
	    throw runtime_error("send() only did a partial write?!");
    }

    cout << "tcp_client: done\n";
    return 0;
}
