#include "Message.h"

Message::Message(int fromNodeId, int toNodeId) :
	fromNodeId(fromNodeId), toNodeId(toNodeId) {
}

Message::Message(int fromNodeId, int toNodeId, const std::map<int, NodeState>& data) :
	fromNodeId(fromNodeId), toNodeId(toNodeId), data(data) {
}

int Message::getFromNodeId() const {
	return fromNodeId;
}

int Message::getToNodeId() const {
	return toNodeId;
}

bool Message::hasEmptyData() const {
	return data.empty();
}

const std::map<int, NodeState>& Message::getData() const {
	return data;
}

void Message::insertData(int nodeId, signed char status) {
	data.insert(std::make_pair(nodeId, NodeState(nodeId, 1, status)));
}

void Message::addPositions(const std::map<int, NodeState>& update) {
	// add this information to our message
	for (auto const& updatePos : update) {
		if (updatePos.first != this->toNodeId) {
			// don't tell a node about itself
			std::map<int, NodeState>::iterator msgIt = data.find(updatePos.first);
			if (msgIt != data.end() && msgIt->first) {
				// we already had data about this node going in this message
				msgIt->second.updateStateIfTimeStampIsHigher(updatePos.second);
			} else {
				data.insert(std::make_pair(updatePos.first, updatePos.second));
			}
		}
	}
}

void Message::subPositions(const std::map<int, NodeState>& received) {
	// we received this information from this node, so no need to send it
	std::map<int, NodeState>::const_iterator rcvIt;
	for (rcvIt = received.begin(); rcvIt != received.end(); rcvIt++) {
		if (rcvIt->first != this->toNodeId) {
			std::map<int, NodeState>::iterator msgIt = this->data.find(rcvIt->first);
			if ((msgIt != this->data.end())
					&& (rcvIt->second.getTimeStamp() >= msgIt->second.getTimeStamp())) {
				this->data.erase(msgIt); // The node doesn't need the data
			}
		}
	}
}
