#include <zmq.hpp>
#include <iostream>

using namespace std;

int main (int argc, char **argv)
{
    if (argc != 2)  {
	cerr << "usage: zmq_pull_server tcp://*:6666\n";
	exit(2);
    }

    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_PULL);
    receiver.bind(argv[1]);

    cout << "calling recv\n";

    zmq::message_t msg;
    receiver.recv(&msg);

    cout << "received message of length " << msg.size() << "\n";

    return 0;
}
