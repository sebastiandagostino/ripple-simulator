#include "Node.h"
#include <sstream>

namespace patch {
    template <typename T> std::string to_string(const T& n) {
        std::ostringstream stm ;
        stm << n ;
        return stm.str();
    }
}

extern int nodesPositive;
extern int nodesNegative;

Node::Node(int nodeId, int mm, int latency) :
	nodeId(nodeId), latency(latency), nodeTimeStamps(mm, 0), nodeStates(mm, 0), messagesSent(0), messagesReceived(0) {
}

int Node::getLatency() const {
	return latency;
}

int Node::getMessagesSent() const {
	return messagesSent;
}

void Node::decreaseMessagesSent() {
	this->messagesSent--;
}

std::vector<int>& Node::getUniqueNodeList() {
	return uniqueNodeList;
}

std::vector<Link>& Node::getLinks() {
	return links;
}

std::vector<int>& Node::getNodeTimeStamps() {
	return nodeTimeStamps;
}

std::vector<signed char>& Node::getNodeStates() {
	return nodeStates;
}

bool Node::isOnUNL(int nodeId) const {
	for (int v : this->uniqueNodeList) {
		if (v == nodeId) {
			return true;
		}
	}
	return false;
}

bool Node::hasLinkTo(int nodeId) const {
	for (const Link& link : links) {
		if (link.getToNodeId() == nodeId) {
			return true;
		}
	}
	return false;
}

std::string Node::toString() {
	// Begin
	std::string string = "[ ";
	// NodeId
	string += patch::to_string(nodeId) + "; ";
	// UNL
	int size = uniqueNodeList.size();
	for (int i = 0; i < size; i++) {
		string += patch::to_string(uniqueNodeList[i]);
		if (i != size - 1) {
			string += ", ";
		}
	}
	string += "; ";
	// Links
	size = links.size();
	for (int i = 0; i < size; i++) {
		string += patch::to_string(links[i].getToNodeId());
		if (i != size - 1) {
			string += ", ";
		}
	}
	// End
	string += " ]";
	return string;
}

void Node::receiveMessage(const Message& message, Network& network) {
	messagesReceived++;

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

	std::map<int, NodeState>::const_iterator chgIt;
	for (chgIt = message.getData().begin(); chgIt != message.getData().end(); chgIt++) {
		if ((chgIt->first != nodeId)
				&& (nodeStates[chgIt->first] != chgIt->second.getState())
				&& (chgIt->second.getTimeStamp() > nodeTimeStamps[chgIt->first])) {
			// This gives us new information about a node
			nodeStates[chgIt->first] = chgIt->second.getState();
			nodeTimeStamps[chgIt->first] = chgIt->second.getTimeStamp();
			changes.insert(std::make_pair(chgIt->first, chgIt->second));
		}
	}

	if (changes.empty()) {
		return; // nothing changed
	}

	// 2) Choose our position change, if any
	int unlCount = 0;
	int unlBalance = 0;
	for (int node : uniqueNodeList) {
		if (nodeStates[node] == 1) {
			unlCount++;
			unlBalance++;
		}
		if (nodeStates[node] == -1) {
			unlCount++;
			unlBalance--;
		}
	}

	if (nodeId < NUM_MALICIOUS_NODES)  {
		// if we are a malicious node, be contrarian
		unlBalance = -unlBalance;
	}

	// add a bias in favor of 'no' as time passes (agree to disagree)
	unlBalance -= network.getMasterTime() / 250;

	bool positionChange = false;
	if (unlCount >= UNL_THRESH) { // We have enough data to make decisions
		if ((nodeStates[nodeId] == 1) && (unlBalance < (-SELF_WEIGHT))) {
			// we switch to -
			nodeStates[nodeId] = -1;
			nodesPositive--;
			nodesNegative++;
			changes.insert(std::make_pair(nodeId, NodeState(nodeId, ++nodeTimeStamps[nodeId], -1)));
			positionChange = true;
		} else if ((nodeStates[nodeId] == -1) && (unlBalance > SELF_WEIGHT)) {
			// we switch to +
			nodeStates[nodeId] = 1;
			nodesPositive++;
			nodesNegative--;
			changes.insert(std::make_pair(nodeId, NodeState(nodeId, ++nodeTimeStamps[nodeId], +1)));
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
				network.sendMessage(Message(nodeId, link.getToNodeId(), changes), link, sendTime);
				messagesSent++;
			}
		}
	}

}
