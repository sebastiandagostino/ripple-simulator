#include <iostream>
#include <fstream>
#include <random>

#include "src/Link.h"
#include "src/Node.h"

#define DEFAULT_FILE "network.txt"

#define NUM_NODES             1000

// Latencies in milliseconds
// E2C - End to core, the latency from a node to a nearby node
// C2C - Core to core, the additional latency when nodes are far
#define MIN_E2C_LATENCY          5
#define MAX_E2C_LATENCY         50
#define MIN_C2C_LATENCY          5
#define MAX_C2C_LATENCY        200

#define NUM_OUTBOUND_LINKS      10

int main(int argc, char* argv[]) {

	std::ofstream file;
	std::string fileName = DEFAULT_FILE;

	if (argc == 2) {
		fileName = argv[1];
	}

	file.open(fileName.c_str());

	// This will produce the same results each time
	std::mt19937 gen; // Standard mersenne_twister_engine
	std::uniform_int_distribution<> r_e2c(MIN_E2C_LATENCY, MAX_E2C_LATENCY);
	std::uniform_int_distribution<> r_c2c(MIN_C2C_LATENCY, MAX_C2C_LATENCY);
	std::uniform_int_distribution<> r_unl(UNL_MIN, UNL_MAX);
	std::uniform_int_distribution<> r_node(0, NUM_NODES - 1);

	Node* nodes[NUM_NODES];

	// create nodes
	std::cout << "Creating nodes" << std::endl;
	for (int i = 0; i < NUM_NODES; i++) {
		nodes[i] = new Node(i, NUM_NODES, r_e2c(gen));

		// our own position starts as 50/50 split
		if (i % 2) {
			nodes[i]->getNodeStates()[i] = 1;
			nodes[i]->getNodeTimeStamps()[i] = 1;
			nodes[i]->setVote(1); // positive vote
		} else {
			nodes[i]->getNodeStates()[i] = -1;
			nodes[i]->getNodeTimeStamps()[i] = 1;
			nodes[i]->setVote(-1); // negative vote
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
	std::cout << "Creating links" << std::endl;
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

	// print to file
	std::cout << "Writing generated network to file: " << fileName << std::endl;
	for (Node* node : nodes) {
		file << node->toString() << std::endl;
	}

}
