#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <sparsehash/sparse_hash_map>

#include "NodeState.h"

struct eqint {
    bool operator()(int i1, int i2) const {
        return i1 == i2;
    }
};

/**
 * A message sent from one node to another, containing the positions taken
 */
class Message {

private:

    int fromNodeId;

    int toNodeId;

    google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint> data;

public:

    Message(int fromNodeId, int toNodeId);

    Message(int fromNodeId, int toNodeId, google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>& data);

    int getFromNodeId() const;

    int getToNodeId() const;

    bool hasEmptyData() const;

    void insertData(int nodeId, signed char status);

    const google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>& getData() const;

    void addPositions(const google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>& data);

    void subPositions(const google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint>& data);

};

#endif /* MESSAGE_H_ */
