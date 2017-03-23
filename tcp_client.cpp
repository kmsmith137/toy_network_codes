#include <unistd.h>
#include <arpa/inet.h>

#include <vector>
#include <cstring>
#include <cassert>
#include <iostream>

#include "time_inlines.hpp"
#include "lexical_cast.hpp"

using namespace std;


static void usage()
{
    cerr << "usage: tcp_client <ip_addr> <port>\n"
	 << "    -t TIME_SECONDS (default 10)\n";

    exit(2);
}


int main(int argc, char **argv)
{
    // Write data in 1MB chunks
    static constexpr int chunk_size = 1024 * 1024;

    vector<string> args;
    double time_seconds = 10.0;

    // Low-budget command-line parsing
    for (int i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    args.push_back(argv[i]);
	    continue;
	}

	if (i == argc-1)
	    usage();

	bool ret = false;

	if (!strcmp(argv[i], "-t"))
	    ret = lexical_cast<double> (argv[i+1], time_seconds);
	else
	    usage();

	if (!ret)
	    usage();

	i++;   // advance by extra token
    }

    if (args.size() != 2)
	usage();

    string ip_addr = args[0];
    int tcp_port = lexical_cast<int> (args[1], "tcp_port");

    assert(time_seconds > 0.0);
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
    cout << "tcp_client: writing to " << ip_addr << ":" << tcp_port << endl;

    bool flag = false;
    struct timeval tv_ini = xgettimeofday();

    while (!flag || (secs_between(tv_ini, xgettimeofday()) < time_seconds)) {
	ssize_t n = send(sockfd, &chunk[0], chunk_size, 0);
	if (n < 0)
	    throw runtime_error(string("send() failed: ") + strerror(errno));
	if (n != chunk_size)
	    throw runtime_error("send() only did a partial write?!");
    }

    cout << "tcp_client: done\n";
    return 0;
}
