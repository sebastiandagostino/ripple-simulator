#include "Message.h"

Message::Message(int from, int to) :
		from_node(from), to_node(to) {
	;
}

Message::Message(int from, int to, const std::map<int, NodeState>& d) :
		from_node(from), to_node(to), data(d) {
	;
}

void Message::addPositions(const std::map<int, NodeState>& update) {
	// add this information to our message
	for (std::map<int, NodeState>::const_iterator update_iterator =
			update.begin(); update_iterator != update.end();
			++update_iterator) {
		if (update_iterator->first != to_node) {
			// don't tell a node about itself
			std::map<int, NodeState>::iterator message_iterator = data.find(
					update_iterator->first);

			if (message_iterator != data.end() && message_iterator->first) {
				// we already had data about this node going in this message
				if (update_iterator->second.ts > message_iterator->second.ts) {
					message_iterator->second.ts = update_iterator->second.ts;
					message_iterator->second.state =
							update_iterator->second.state;
				}
			} else
				data.insert(
						std::make_pair(update_iterator->first,
								update_iterator->second));
		}
	}
}

void Message::subPositions(const std::map<int, NodeState>& received) {
	// we received this information from this node, so no need to send it
	for (std::map<int, NodeState>::const_iterator received_iterator =
			received.begin(); received_iterator != received.end();
			++received_iterator) {
		if (received_iterator->first != to_node) {
			std::map<int, NodeState>::iterator message_iterator = data.find(
					received_iterator->first);
			if ((message_iterator != data.end())
					&& (received_iterator->second.ts
							>= message_iterator->second.ts)) {
				data.erase(message_iterator); // The node doesn't need the data
			}
		}
	}
}
