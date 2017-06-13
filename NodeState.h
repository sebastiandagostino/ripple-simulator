#ifndef NODESTATE_H_
#define NODESTATE_H_

/**
 * A NodeState as propagated by the network
 */
class NodeState {

public:

	int node;

	int ts;

	unsigned char state;

	NodeState(int n, int t, unsigned char s);

};

#endif /* NODESTATE_H_ */
