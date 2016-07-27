// g++ -std=c++11 -pthread -fPIC -Wall -O3 -o udp_client udp_client.cpp

#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <vector>
#include <iostream>

using namespace std;


inline struct timeval xgettimeofday()
{
    struct timeval tv;

    int err = gettimeofday(&tv, NULL);
    if (err)
	throw runtime_error("gettimeofday failed");

    return tv;
}


inline double usec_between(struct timeval &tv1, struct timeval &tv2)
{
    return 1.0e6*(tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec);
}


int main(int argc, char **argv)
{
    static constexpr double target_gbps = 1.0;
    static constexpr int nbytes_per_packet = 4096;
    static constexpr int npackets_tot = 100000;
    static constexpr int udp_port = 13299;
    static const char *addr = "127.0.0.1";

    static constexpr double usec_per_packet = 1.0e6 * (8.0e-9 * nbytes_per_packet) / target_gbps;

    vector<uint8_t> packet(nbytes_per_packet, 0);

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(udp_port);

    int err = inet_pton(AF_INET, addr, &saddr.sin_addr);
    if (err <= 0)
	throw runtime_error("ch_frb_io: inet_pton() failed (note that no DNS lookup is done, the argument must be a numerical IP address)");

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        throw runtime_error(string("socket() failed: ") + strerror(errno));

    // Note: bind() not called, so source port number of outgoing packets will be arbitrarily assigned

    if (connect(sockfd, reinterpret_cast<struct sockaddr *> (&saddr), sizeof(saddr)) < 0) {
        close(sockfd);
        throw runtime_error(string("connect() failed: ") + strerror(errno));
    }

    struct timeval tv_ini = xgettimeofday();
    struct timeval tv_prev = tv_ini;
    double usec_sleeping = 0.0;

    for (int ipacket = 0; ipacket < npackets_tot; ipacket++) {
	struct timeval tv_curr = xgettimeofday();

	// how many usec in the future should we sent the packet?
	double t1 = (usec_per_packet * ipacket) - usec_between(tv_ini, tv_curr);
	double t2 = (0.5 * usec_per_packet) - usec_between(tv_prev, tv_curr);
	double t = max(t1, t2);
	
	if (t >= 1.0) {
	    int err = usleep(t);
	    if (err)
		throw runtime_error("usleep failed");
	    usec_sleeping += t;
	}

	ssize_t n = send(sockfd, &packet[0], nbytes_per_packet, 0);
	if (n < 0)
	    throw runtime_error(string("send() failed: ") + strerror(errno));
	if (n != nbytes_per_packet)
	    throw runtime_error("send() only did a partial write?!");

	tv_prev = tv_curr;
    }

    struct timeval tv_end = xgettimeofday();
    double usec_elapsed = usec_between(tv_ini, tv_end);
    double actual_gbps = (8.0e-9 * npackets_tot * nbytes_per_packet) / (1.0e-6 * usec_elapsed);


    cout << "udp_client: wrote " << (npackets_tot * nbytes_per_packet) << " bytes in " << (usec_elapsed/1.0e6) << " secs\n"
	 << "   target_gpbs = " << target_gbps << "\n"
	 << "   actual_gpbs = " << actual_gbps << "\n"
	 << "   sleepfrac = " << (usec_sleeping/usec_elapsed) << "\n";

    return 0;
}
