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

void NodeState::setTimeStamp(int timeStamp) {
	this->timeStamp = timeStamp;
}

int NodeState::getTimeStamp() const {
	return timeStamp;
}

void NodeState::setState(unsigned char state) {
	this->state = state;
}

void NodeState::updateStateIfTimeStampIsHigher(const NodeState& nodeState) {
	if (nodeState.getTimeStamp() > this->getTimeStamp()) {
		this->setTimeStamp(nodeState.getTimeStamp());
		this->setState(nodeState.getState());
	}
}
