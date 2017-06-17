#ifndef NODESTATE_H_
#define NODESTATE_H_

/**
 * A NodeState as propagated by the network
 */
class NodeState {

private:

	int nodeId;

	int timeStamp;

	unsigned char state;


public:

	NodeState(int nodeId, int timeStamp, unsigned char state);

	int getNodeId() const;

	int getTimeStamp() const;

	unsigned char getState() const;

	void updateStateIfTimeStampIsHigher(const NodeState& nodeState);

};

#endif /* NODESTATE_H_ */
