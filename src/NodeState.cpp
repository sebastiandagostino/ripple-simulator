#include "NodeState.h"

NodeState::NodeState(int nodeId, int timeStamp, unsigned char state) :
	nodeId(nodeId), timeStamp(timeStamp), state(state) {
}

int NodeState::getNodeId() const {
	return nodeId;
}

unsigned char NodeState::getState() const {
	return state;
}

int NodeState::getTimeStamp() const {
	return timeStamp;
}

void NodeState::updateStateIfTimeStampIsHigher(const NodeState& nodeState) {
	if (this->nodeId == nodeState.getNodeId()
			&& nodeState.getTimeStamp() > this->getTimeStamp()) {
		this->timeStamp = nodeState.getTimeStamp();
		this->state = nodeState.getState();
	}
}
