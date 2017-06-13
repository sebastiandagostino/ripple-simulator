#ifndef NODE_H_
#define NODE_H_

#include <vector>

#include "Link.h"
#include "Message.h"
#include "Network.h"

#define NUM_MALICIOUS_NODES     15

#define UNL_MIN                 20

#define UNL_MAX                 30

#define UNL_THRESH              (UNL_MIN/2) // unl datapoints we have to have before we change position

#define BASE_DELAY               1 	// extra time we delay a message to coalesce/suppress

#define SELF_WEIGHT              1 	// how many UNL votes you give yourself

#define PACKETS_ON_WIRE          3 	// how many packets can be "on the wire" per link per direction
									// simulates non-infinite bandwidth

class Node {

public:

    int n, e2c_latency;

    std::vector<int> unl;
    std::vector<Link> links;

    std::vector<int> nts; // node time stamps
    std::vector<signed char> knowledge; // node states

    int messages_sent, messages_received;

    Node(int nn, int mm);

    void processMessage(const Message& m);

    bool isOnUNL(int j);

    bool hasLinkTo(int j);

    void receiveMessage(const Message& m, Network& n);

};

#endif /* NODE_H_ */
