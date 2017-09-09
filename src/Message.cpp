#include "Message.h"

Message::Message(int fromNodeId, int toNodeId) :
    fromNodeId(fromNodeId), toNodeId(toNodeId), data() {
	data.clear_deleted_key();
	data.set_deleted_key(-1);
}

Message::Message(int fromNodeId, int toNodeId, google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>& data) :
    fromNodeId(fromNodeId), toNodeId(toNodeId), data(data) {
	data.clear_deleted_key();
	data.set_deleted_key(-1);
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

const google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>& Message::getData() const {
    return data;
}

void Message::insertData(int nodeId, signed char status) {
    data[nodeId] = NodeState(nodeId, 1, status);
}

void Message::addPositions(const google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>& update) {
    // add this information to our message
    for (auto const& updateState : update) {
        if (updateState.first != this->toNodeId) {
            // don't tell a node about itself
            google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>::iterator msgIt = data.find(updateState.first);
            if (msgIt != data.end() && msgIt->first) {
                // we already had data about this node going in this message
                msgIt->second.updateStateIfTimeStampIsHigher(updateState.second);
            } else {
                data[updateState.first] = updateState.second;
            }
        }
    }
}

void Message::subPositions(const google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>& received) {
    // we received this information from this node, so no need to send it
    for (auto const& receivedState : received) {
        if (receivedState.first != this->toNodeId) {
            google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>::iterator msgIt = data.find(receivedState.first);
            if ((msgIt != this->data.end()) && (receivedState.second.getTimeStamp() >= msgIt->second.getTimeStamp())) {
                data.erase(msgIt->first); // The node doesn't need the data
            }
        }
    }
}
