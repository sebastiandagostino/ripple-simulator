#include <iostream>
#include <fstream>
#include <random>

#include "src/Link.h"
#include "src/Node.h"

#include "lib/jsmn.h"

#define DEFAULT_FILE "network.txt"

#define NUM_NODES             1000

#define UNL_MIN                 20
#define UNL_MAX                 30

#define NUM_OUTBOUND_LINKS      10

// Latencies in milliseconds
// E2C - End to core, the latency from a node to a nearby node
// C2C - Core to core, the additional latency when nodes are far
#define MIN_E2C_LATENCY          5
#define MAX_E2C_LATENCY         50
#define MIN_C2C_LATENCY          5
#define MAX_C2C_LATENCY        200

int main(int argc, char* argv[]) {

	std::ofstream file;
	std::string fileName = DEFAULT_FILE;

	int numNodes = NUM_NODES;
	int numOutboundLinks = NUM_OUTBOUND_LINKS;

	if (argc == 2) {
		fileName = argv[1];
	}

	file.open(fileName.c_str());

	if (!file.is_open()) {
		return -1;
	}

	std::cout << "Running with NUM_NODES = " << numNodes << std::endl;
	std::cout << "Running with UNL_MIN = " << UNL_MIN << std::endl;
	std::cout << "Running with UNL_MAX = " << UNL_MAX << std::endl;
	std::cout << "Running with NUM_OUTBOUND_LINKS = " << numOutboundLinks << std::endl;
	std::cout << "Running with MIN_E2C_LATENCY = " << MIN_E2C_LATENCY << std::endl;
	std::cout << "Running with MAX_E2C_LATENCY = " << MAX_E2C_LATENCY << std::endl;
	std::cout << "Running with MIN_C2C_LATENCY = " << MIN_C2C_LATENCY << std::endl;
	std::cout << "Running with MAX_C2C_LATENCY = " << MAX_C2C_LATENCY << std::endl;
	std::cout << std::endl;

	file << numNodes << std::endl;
	file << UNL_MIN << std::endl;
	file << UNL_MAX << std::endl;
	file << numOutboundLinks << std::endl;
	file << MIN_E2C_LATENCY << std::endl;
	file << MAX_E2C_LATENCY << std::endl;
	file << MIN_C2C_LATENCY << std::endl;
	file << MAX_C2C_LATENCY << std::endl;

	// This will produce the same results each time
	std::mt19937 gen; // Standard mersenne_twister_engine
	std::uniform_int_distribution<> r_e2c(MIN_E2C_LATENCY, MAX_E2C_LATENCY);
	std::uniform_int_distribution<> r_c2c(MIN_C2C_LATENCY, MAX_C2C_LATENCY);
	std::uniform_int_distribution<> r_unl(UNL_MIN, UNL_MAX);
	std::uniform_int_distribution<> r_node(0, numNodes - 1);

	Node* nodes[numNodes];

	// create nodes
	std::cout << "Creating nodes" << std::endl;
	for (int i = 0; i < numNodes; i++) {
		nodes[i] = new Node(i, numNodes, r_e2c(gen));

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
	for (int i = 0; i < numNodes; i++) {
		int links = numOutboundLinks;
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
	file.close();

	return 0;

}
