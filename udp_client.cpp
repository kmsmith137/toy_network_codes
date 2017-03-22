#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <vector>
#include <cstring>
#include <cassert>
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


inline double usec_between(const struct timeval &tv1, const struct timeval &tv2)
{
    return 1.0e6*(tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec);
}


static void usage()
{
    cerr << "usage: udp_client <ip_addr> <port>\n"
	 << "with flags as follows:\n"
	 << "    -g GIGABITS_PER_SECOND (default 0.95)\n"
	 << "    -n NBYTES_PER_PACKET (default 1500)\n"
	 << "    -t TIMEOUT_SECONDS (default 10)\n";

    exit(2);
}


int main(int argc, char **argv)
{
    vector<string> args;
    int nbytes_per_packet = 1500;
    double timeout = 10.0;
    double gbps = 0.95;

    // Low-budget command-line parsing
    for (int i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    args.push_back(argv[i]);
	    continue;
	}

	if (i == argc-1)
	    usage();

	bool ret = false;

	if (!strcmp(argv[i], "-g"))
	    ret = lexical_cast<double> (argv[i+1], gbps);
	else if (!strcmp(argv[i], "-n"))
	    ret = lexical_cast<int> (argv[i+1], nbytes_per_packet);
	else if (!strcmp(argv[i], "-t"))
	    ret = lexical_cast<double> (argv[i+1], timeout);
	else
	    usage();

	if (!ret)
	    usage();

	i++;   // advance by extra token
    }

    if (args.size() != 2)
	usage();

    string ip_addr = args[0];
    int udp_port = lexical_cast<int> (args[1], "udp_port");

    assert(gbps > 0.0);
    assert(timeout > 0.0);
    assert(nbytes_per_packet > 0 && nbytes_per_packet <= 65536);
    assert(udp_port > 0 && udp_port < 65536);

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(udp_port);

    int err = inet_pton(AF_INET, ip_addr.c_str(), &saddr.sin_addr);
    if (err <= 0)
	throw runtime_error(ip_addr + ": inet_pton() failed (note that no DNS lookup is done, the argument must be a numerical IP address)");

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        throw runtime_error(string("socket() failed: ") + strerror(errno));

    // Note: bind() not called, so source port number of outgoing packets will be arbitrarily assigned

    if (connect(sockfd, reinterpret_cast<struct sockaddr *> (&saddr), sizeof(saddr)) < 0) {
        close(sockfd);
        throw runtime_error(string("connect() failed: ") + strerror(errno));
    }

    int npackets = int(timeout * gbps / (8.0e-9 * nbytes_per_packet)) + 1;
    vector<uint8_t> packet(nbytes_per_packet, 0);

    cout << "udp_client: writing " << npackets 
	 << " packets to " << ip_addr << ":" << udp_port 
	 << " (gbps=" << gbps << ", nbytes_per_packet=" << nbytes_per_packet << ")" << endl;

    struct timeval tv_ini = xgettimeofday();

    for (int ipacket = 0; ipacket < npackets; ipacket++) {
	// Microseconds
	double dt = usec_between(tv_ini, xgettimeofday());
	double dt_target = 8.0e-3 * nbytes_per_packet * ipacket / gbps;

	if (dt_target > dt + 1.0) {
	    int err = usleep(dt_target - dt);
	    if (err)
		throw runtime_error("usleep failed");
	}

	ssize_t n = send(sockfd, &packet[0], nbytes_per_packet, 0);
	if (n < 0)
	    throw runtime_error(string("send() failed: ") + strerror(errno));
	if (n != nbytes_per_packet)
	    throw runtime_error("send() only did a partial write?!");
    }

    double usec_elapsed = usec_between(tv_ini, xgettimeofday());
    double actual_gbps = (8.0e-3 * npackets * nbytes_per_packet) / usec_elapsed;

    cout << "udp_client: wrote " << npackets << " packets in " << (usec_elapsed/1.0e6) << " secs\n"
	 << "   target_gpbs = " << gbps << "\n"
	 << "   actual_gpbs = " << actual_gbps << "\n";

    return 0;
}
