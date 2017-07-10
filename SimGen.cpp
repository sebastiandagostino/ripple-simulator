#include <iostream>
#include <random>

#include "src/Event.h"
#include "src/Link.h"
#include "src/Message.h"
#include "src/Network.h"
#include "src/Node.h"
#include "src/NodeState.h"

#define NUM_NODES             1000
#define CONSENSUS_PERCENT       80

// Latencies in milliseconds
// E2C - End to core, the latency from a node to a nearby node
// C2C - Core to core, the additional latency when nodes are far
#define MIN_E2C_LATENCY          5
#define MAX_E2C_LATENCY         50
#define MIN_C2C_LATENCY          5
#define MAX_C2C_LATENCY        200

#define NUM_OUTBOUND_LINKS      10

int nodesPositive = 0;
int nodesNegative = 0;

int main(void) {

	// This will produce the same results each time
	std::mt19937 gen; // Standard mersenne_twister_engine
	std::uniform_int_distribution<> r_e2c(MIN_E2C_LATENCY, MAX_E2C_LATENCY);
	std::uniform_int_distribution<> r_c2c(MIN_C2C_LATENCY, MAX_C2C_LATENCY);
	std::uniform_int_distribution<> r_unl(UNL_MIN, UNL_MAX);
	std::uniform_int_distribution<> r_node(0, NUM_NODES - 1);

	Node* nodes[NUM_NODES];

	// create nodes
	std::cerr << "Creating nodes" << std::endl;
	for (int i = 0; i < NUM_NODES; i++) {
		nodes[i] = new Node(i, NUM_NODES, r_e2c(gen));

		// our own position starts as 50/50 split
		if (i % 2) {
			nodes[i]->getNodeStates()[i] = 1;
			nodes[i]->getNodeTimeStamps()[i] = 1;
			nodesPositive++;
		} else {
			nodes[i]->getNodeStates()[i] = -1;
			nodes[i]->getNodeTimeStamps()[i] = 1;
			nodesNegative++;
		}

		// build our UNL
		int unl_count = r_unl(gen);
		while (unl_count > 0) {
			int cn = r_node(gen);
			if ((cn != i) && !nodes[i]->isOnUNL(cn)) {
				nodes[i]->getUniqueNodeList().push_back(cn);
				unl_count--;
			}
		}
	}

	// create links
	std::cerr << "Creating links" << std::endl;
	for (int i = 0; i < NUM_NODES; i++) {
		int links = NUM_OUTBOUND_LINKS;
		while (links > 0) {
			int lt = r_node(gen);
			if ((lt != i) && !nodes[i]->hasLinkTo(lt)) {
				int ll = nodes[i]->getLatency() + nodes[lt]->getLatency() + r_c2c(gen);
				nodes[i]->getLinks().push_back(Link(lt, ll));
				nodes[lt]->getLinks().push_back(Link(i, ll));
				links--;
			}
		}
	}

	for (Node* node : nodes) {
		std::cout << node->toString() << std::endl;
	}

}
