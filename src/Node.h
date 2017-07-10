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

private:

    int nodeId;

    int latency; // E2C - End to core latency, the latency from a node to a nearby node

    std::vector<int> uniqueNodeList;

    std::vector<Link> links;

    std::vector<int> nodeTimeStamps;

    std::vector<signed char> nodeStates;

    int messagesSent;

    int messagesReceived;

    int vote;

public:

    Node(int nodeId, int mm, int latency);

	int getLatency() const;

	int getMessagesSent() const;

	void decreaseMessagesSent();

	std::vector<int>& getUniqueNodeList();

    std::vector<Link>& getLinks();

    std::vector<int>& getNodeTimeStamps();

    std::vector<signed char>& getNodeStates();

    bool isOnUNL(int nodeId) const;

    bool hasLinkTo(int nodeId) const;

	int getVote() const;

	void setVote(int vote);

    std::string toString();

    void receiveMessage(const Message& message, Network& network);

};

#endif /* NODE_H_ */
