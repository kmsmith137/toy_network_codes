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
    cerr << "usage: tcp_server <ip_addr> <port>\n";
    exit(2);
}


int main(int argc, char **argv)
{
    // Read data in 1MB chunks
    static constexpr int chunk_size = 1024 * 1024;

    if (argc != 3)
	usage();

    string ip_addr = argv[1];
    int tcp_port = lexical_cast<int> (argv[2], "tcp_port");

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
	throw runtime_error(string("socket() failed: ") + strerror(errno));

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(tcp_port);

    int err = inet_pton(AF_INET, ip_addr.c_str(), &saddr.sin_addr);
    if (err <= 0)
	throw runtime_error(ip_addr + ": inet_pton() failed (note that no DNS lookup is done, the argument must be a numerical IP address)");

    err = ::bind(sockfd, (struct sockaddr *) &saddr, sizeof(saddr));
    if (err < 0)
	throw runtime_error(string("bind() failed: ") + strerror(errno));

    cout << "tcp_server listening on " << ip_addr << ":" << tcp_port << "\n";

    err = listen(sockfd, 10);
    if (err < 0)
	throw runtime_error(string("listen() failed: ") + strerror(errno));

    struct sockaddr_in saddr2;
    memset(&saddr2, 0, sizeof(saddr2));

    socklen_t saddr2_len = sizeof(saddr2);
    std::vector<uint8_t> chunk(chunk_size, 0);

    for (;;) {
	int sockfd2 = accept(sockfd, (sockaddr *) &saddr2, &saddr2_len);
	if (sockfd2 < 0)
	    throw runtime_error(string("accept() failed: ") + strerror(errno));

	cout << "accepted connection\n";

	struct timeval tv_ini = xgettimeofday();
	ssize_t ntot = 0;

	for (;;) {
	    ssize_t n = read(sockfd2, (char *) &chunk[0], chunk_size);
	    if (n < 0)
		throw runtime_error(string("read() failed: ") + strerror(errno));
	    if (n == 0)
		break;
	    ntot += n;
	}

	double dt = secs_between(tv_ini, xgettimeofday());
	double gbps = 8.0e-9 * ntot / dt;

	cout << "tcp_server: read " << ntot << " bytes in " << dt << " secs, gbps=" << gbps << endl;
    }

    return 0;
}
