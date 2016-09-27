// g++ -Wall -o dns-lookup dns-lookup.cpp 

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    if (argc != 2) {
	cerr << "usage: dns-lookup <hostname>\n";
	exit(2);
    }

    const char *hostname = argv[1];

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;

#if 0
    int err = inet_pton(AF_INET, argv[1], &saddr.sin_addr);
    if (err == 0)
	throw runtime_error("couldn't resolve hostname '" + string(hostname) + "' to an IP address: general parse error");
    if (err < 0)
        throw runtime_error("couldn't resolve hostname '" + string(hostname) + "' to an IP address: " + strerror(errno) + "general parse error");
#endif

    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_flags = AI_DEFAULT;
    hint.ai_family = AF_INET;   // IPv4
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_protocol = IPPROTO_UDP;

    struct addrinfo *result = nullptr;

    int err = getaddrinfo(hostname, NULL, &hint, &result);
    if (err)
	throw runtime_error("getaddrinfo: " + string(gai_strerror(err)));
    if (!result)
	throw runtime_error("internal error: getaddrinfo() returned success, but result pointer is non-NULL");
    
    for (struct addrinfo *p = result; p; p = p->ai_next) {
	struct sockaddr_in *q = (struct sockaddr_in *) p->ai_addr;

	vector<char> buf(INET_ADDRSTRLEN+1, 0);
	if (!inet_ntop(AF_INET, &q->sin_addr, &buf[0], INET_ADDRSTRLEN))
	    throw runtime_error("couldn't convert IP address to a string representation: " + string(strerror(errno)));

	cout << hostname << " -> " << &buf[0] << endl;
    }

    freeaddrinfo(result);

    return 0;
}
    
