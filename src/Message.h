#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <sparsehash/sparse_hash_map>

#include "NodeState.h"

struct eqint {
    bool operator()(int i1, int i2) const {
        return i1 == i2;
    }
};

typedef google::sparse_hash_map<int, NodeState, std::tr1::hash<int>, eqint> Map;

/**
 * A message sent from one node to another, containing the positions taken
 */
class Message {

private:

    int fromNodeId;

    int toNodeId;

    Map data;

public:

    Message(int fromNodeId, int toNodeId);

    Message(int fromNodeId, int toNodeId, Map& data);

    int getFromNodeId() const;

    int getToNodeId() const;

    bool hasEmptyData() const;

    void insertData(int nodeId, signed char status);

    const Map& getData() const;

    void addPositions(const Map& data);

    void subPositions(const Map& data);

};

#endif /* MESSAGE_H_ */
