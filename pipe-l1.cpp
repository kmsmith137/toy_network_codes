#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace std;

int main(int argc, char **argv)
{
    // EPIPE instead of SIGPIPE
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	throw runtime_error(string("signal() failed: ") + strerror(errno));
    
    // pipefd[0] = "read end" of pipe = L1b
    // pipefd[1] = "write end" of pipe = L1a
    
    int pipefd[2];
    
    if (pipe(pipefd) < 0)
	throw runtime_error(string("pipe() failed: ") + strerror(errno));

    pid_t l1b_pid = fork();
    
    if (l1b_pid < 0)
	throw runtime_error(string("fork() failed: ") + strerror(errno));	

    if (l1b_pid == 0) {
	// Child process.
	close(pipefd[1]);
	
	const char *argv0 = "./pipe-l1b.py";
	string argv1 = to_string(pipefd[0]);
	    
	int err = execl(argv0, argv0, argv1.c_str(), NULL);
	if (err < 0)
	    throw runtime_error(string("exec() failed: ") + strerror(errno));
	
	throw runtime_error("exec() returned?!");
    }

    // Parent process.
    close(pipefd[0]);

#if 0
    int pipe_sz = fcntl(pipefd[1], F_GETPIPE_SZ);
    if (pipe_sz < 0)
	throw runtime_error(string("fcntl(F_GETPIPE_SZ) failed: ") + strerror(errno));
    cout << "getpipe_sz: " << pipe_sz << endl;

    // Can fail with EPERM if pipe_sz is too large
    pipe_sz = 1048576;
    cout << "setpipe_sz: " << pipe_sz << endl;
    if (fcntl(pipefd[1], F_SETPIPE_SZ, pipe_sz) < 0)
	throw runtime_error(string("fcntl(F_GETPIPE_SZ) failed: ") + strerror(errno));

    pipe_sz = fcntl(pipefd[1], F_GETPIPE_SZ);
    if (pipe_sz < 0)
	throw runtime_error(string("fcntl(F_GETPIPE_SZ) failed: ") + strerror(errno));
    cout << "getpipe_sz: " << pipe_sz << endl;
#endif
    
    // To test what happens if L1a is slow to start
    // (Answer: L1b blocks at read())
    // sleep(5);
    
    int nmsg = 10;
    int msg_size = 32768;
    void *msg = malloc(msg_size);
    memset(msg, 0, msg_size);

    for (int i = 0; i < nmsg; i++) {
	cout << "l1a: writing " << msg_size << " bytes" << endl;
	
	int write_err = write(pipefd[1], msg, msg_size);
	
	if (write_err < 0)
	    throw runtime_error(string("write() failed: ") + strerror(errno));
	if (write_err != msg_size)
	    throw runtime_error("write() didn't return expected byte count");

	sleep(1);
    }

    close(pipefd[1]);
    cout << "l1a: waiting for l1b to exit" << endl;
    
    int l1b_status = -1;
    pid_t wait_pid = waitpid(l1b_pid, &l1b_status, 0);

    if (wait_pid < 0)
	throw runtime_error(string("waitpid() failed: ") + strerror(errno));
    if (wait_pid != l1b_pid)
	throw runtime_error("waitpid() returned wrong pid?!");
    if (l1b_status != 0)
	throw runtime_error("l1b child process returned error");
    
    return 0;
}

