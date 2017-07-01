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
    
    int fd = open("myfifo", O_WRONLY);
    if (fd < 0)
	throw runtime_error(string("open() failed: ") + strerror(errno));

    cout << "writing\n";

    const char *msg = "hello world";
    if (write(fd, msg, strlen(msg)) < 0)
	throw runtime_error(string("write() failed: ") + strerror(errno));
    
    return 0;
}
