#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <map>

#include "NodeState.h"

/**
 * A message sent from one node to another, containing the positions taken
 */
class Message {

public:

    int from_node, to_node;

    std::map<int, NodeState> data;

    Message(int from, int to);

    Message(int from, int to, const std::map<int, NodeState>& d);

    void addPositions(const std::map<int, NodeState>&);

    void subPositions(const std::map<int, NodeState>&);

};

#endif /* MESSAGE_H_ */
