#include "Node.h"

extern int nodes_positive;
extern int nodes_negative;

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

void Node::receiveMessage(const Message& m, Network& network) {
	messages_received++;

	// If we were going to send any of this data to that node, skip it
	for (Link& link : links) {
		if ((link.getToNodeId() == m.from_node) && (link.getSendTime() >= network.master_time)) {
			// We can still update a waiting outbound message
			link.getMessages()->subPositions(m.data);
			break;
		}
	}

	// 1) Update our knowledge
	std::map<int, NodeState> changes;

	std::map<int, NodeState>::const_iterator change_it;
	for (change_it = m.data.begin(); change_it != m.data.end(); change_it++) {
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
	int unl_count = 0, unl_balance = 0;
	for (int node : unl) {
		if (knowledge[node] == 1) {
			unl_count++;
			unl_balance++;
		}
		if (knowledge[node] == -1) {
			unl_count++;
			unl_balance--;
		}
	}

	if (n < NUM_MALICIOUS_NODES)  {
		// if we are a malicious node, be contrarian
		unl_balance = -unl_balance;
	}

	// add a bias in favor of 'no' as time passes (agree to disagree)
	unl_balance -= network.master_time / 250;

	bool pos_change = false;
	if (unl_count >= UNL_THRESH) { // We have enough data to make decisions
		if ((knowledge[n] == 1) && (unl_balance < (-SELF_WEIGHT))) {
			// we switch to -
			knowledge[n] = -1;
			nodes_positive--;
			nodes_negative++;
			changes.insert(std::make_pair(n, NodeState(n, ++nts[n], -1)));
			pos_change = true;
		} else if ((knowledge[n] == -1) && (unl_balance > SELF_WEIGHT)) {
			// we switch to +
			knowledge[n] = 1;
			nodes_positive++;
			nodes_negative--;
			changes.insert(std::make_pair(n, NodeState(n, ++nts[n], +1)));
			pos_change = true;
		}
	}

	// 3) Broadcast the message
	for (Link& link : links) {
		if (pos_change || (link.getToNodeId() != m.from_node)) {
			// can we update an unsent message?
			if (link.getSendTime() > network.master_time)
				link.getMessages()->addPositions(changes);
			else {
				// No, we need a new mesage
				int send_time = network.master_time;
				if (!pos_change) {
					// delay the messag a bit to permit coalescing and suppression
					send_time += BASE_DELAY;
					if (link.getReceiveTime() > send_time) // a packet is on the wire
						send_time += link.getTotalLatency() / PACKETS_ON_WIRE; // wait a bit extra to send
				}
				network.sendMessage(Message(n, link.getToNodeId(), changes), link, send_time);
				messages_sent++;
			}
		}
	}

}
