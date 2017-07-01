#include <iostream>
#include <zmq.hpp>

using namespace std;

int main (int argc, char **argv)
{
    if (argc != 2)  {
	cerr << "usage: zmq_push_client tcp://localhost:6666\n";
	exit(2);
    }

    zmq::context_t context(1);
    zmq::socket_t sender(context, ZMQ_PUSH);

    cout << "calling connect()\n";
    sender.connect(argv[1]);

    int msg_len = 10;
    zmq::message_t msg(10);
    memset(msg.data(), 0, msg_len);

    cout << "sending message\n";
    sender.send(msg);

    cout << "done\n";
    return 0;
}
