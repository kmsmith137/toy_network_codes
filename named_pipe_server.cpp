#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <vector>
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace std;


int main(int argc, char **argv)
{
    cout << "opening ./myfifo\n";
    
    int fd = open("myfifo", O_RDONLY);
    if (fd < 0)
	throw runtime_error(string("open() failed: ") + strerror(errno));

    cout << "acquiring lock\n";
    
    // Important, to prevent multiple readers (presumably unintentional)
    if (flock(fd, LOCK_EX | LOCK_NB) < 0)
	throw runtime_error(string("flock() failed: ") + strerror(errno));

    cout << "reading\n";

    char buf[100];

    for (;;) {
        memset(buf, 0, sizeof(buf));
	ssize_t nbytes = read(fd, buf, sizeof(buf-1));
	if (nbytes < 0)
	    throw runtime_error(string("open() failed: ") + strerror(errno));
	else if (nbytes == 0)
	    break;
	else
	    cout << "Received: " << buf << "\n";
    }

    cout << "End of file\n";
    return 0;
}
