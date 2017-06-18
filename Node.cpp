#include "Node.h"

extern int nodesPositive;
extern int nodesNegative;

Node::Node(int nn, int mm) :
	n(nn), nts(mm, 0), knowledge(mm, 0), messages_sent(0), messages_received(0) {
}

bool Node::isOnUNL(int j) {
	for (int v : unl) {
		if (v == j) {
			return true;
		}
	}
	return false;
}

bool Node::hasLinkTo(int j) {
	for (const Link& link : links) {
		if (link.getToNodeId() == j) {
			return true;
		}
	}
	return false;
}

void Node::receiveMessage(const Message& message, Network& network) {
	messages_received++;

	// If we were going to send any of this data to that node, skip it
	for (Link& link : links) {
		if ((link.getToNodeId() == message.getFromNodeId()) && (link.getSendTime() >= network.getMasterTime())) {
			// We can still update a waiting outbound message
			link.getMessages()->subPositions(message.getData());
			break;
		}
	}

	// 1) Update our knowledge
	std::map<int, NodeState> changes;

	std::map<int, NodeState>::const_iterator change_it;
	for (change_it = message.getData().begin(); change_it != message.getData().end(); change_it++) {
		if ((change_it->first != n)
				&& (knowledge[change_it->first] != change_it->second.getState())
				&& (change_it->second.getTimeStamp() > nts[change_it->first])) {
			// This gives us new information about a node
			knowledge[change_it->first] = change_it->second.getState();
			nts[change_it->first] = change_it->second.getTimeStamp();
			changes.insert(std::make_pair(change_it->first, change_it->second));
		}
	}

	if (changes.empty()) {
		return; // nothing changed
	}

	// 2) Choose our position change, if any
	int unlCount = 0;
	int unlBalance = 0;
	for (int node : unl) {
		if (knowledge[node] == 1) {
			unlCount++;
			unlBalance++;
		}
		if (knowledge[node] == -1) {
			unlCount++;
			unlBalance--;
		}
	}

	if (n < NUM_MALICIOUS_NODES)  {
		// if we are a malicious node, be contrarian
		unlBalance = -unlBalance;
	}

	// add a bias in favor of 'no' as time passes (agree to disagree)
	unlBalance -= network.getMasterTime() / 250;

	bool positionChange = false;
	if (unlCount >= UNL_THRESH) { // We have enough data to make decisions
		if ((knowledge[n] == 1) && (unlBalance < (-SELF_WEIGHT))) {
			// we switch to -
			knowledge[n] = -1;
			nodesPositive--;
			nodesNegative++;
			changes.insert(std::make_pair(n, NodeState(n, ++nts[n], -1)));
			positionChange = true;
		} else if ((knowledge[n] == -1) && (unlBalance > SELF_WEIGHT)) {
			// we switch to +
			knowledge[n] = 1;
			nodesPositive++;
			nodesNegative--;
			changes.insert(std::make_pair(n, NodeState(n, ++nts[n], +1)));
			positionChange = true;
		}
	}

	// 3) Broadcast the message
	for (Link& link : links) {
		if (positionChange || (link.getToNodeId() != message.getFromNodeId())) {
			int sendTime = network.getMasterTime();
			// can we update an unsent message?
			if (link.getSendTime() > sendTime) {
				link.getMessages()->addPositions(changes);
			} else {
				// No, we need a new message
				if (!positionChange) {
					// delay the message a bit to permit coalescing and suppression
					sendTime += BASE_DELAY;
					if (link.getReceiveTime() > sendTime) // a packet is on the wire
						sendTime += link.getTotalLatency() / PACKETS_ON_WIRE; // wait a bit extra to send
				}
				network.sendMessage(Message(n, link.getToNodeId(), changes), link, sendTime);
				messages_sent++;
			}
		}
	}

}
