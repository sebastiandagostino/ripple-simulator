#include <iostream>
#include <fstream>
#include <random>

#include "src/Link.h"
#include "src/Node.h"

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
    std::string fileName;

    int numNodes = NUM_NODES;
    int numOutboundLinks = NUM_OUTBOUND_LINKS;

    if (argc == 2) {
        fileName = argv[1];
    } else {
        std::cerr << "usage: gen output_file" << std::endl;
        return EXIT_FAILURE;
    }

    file.open(fileName.c_str());

    if (!file.is_open()) {
        std::cerr << "Unable to create file. Exiting..." << std::endl;
        return EXIT_FAILURE;
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

    // This will produce the same results each time
    std::mt19937 gen; // Standard mersenne_twister_engine
    std::uniform_int_distribution<> r_e2c(MIN_E2C_LATENCY, MAX_E2C_LATENCY);
    std::uniform_int_distribution<> r_c2c(MIN_C2C_LATENCY, MAX_C2C_LATENCY);
    std::uniform_int_distribution<> r_unl(UNL_MIN, UNL_MAX);
    std::uniform_int_distribution<> r_node(0, numNodes - 1);

    Node* nodes[numNodes];

    // Create nodes
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

        // Build our UNL
        int unl_count = r_unl(gen);
        while (unl_count > 0) {
            int cn = r_node(gen);
            if ((cn != i) && !nodes[i]->isOnUNL(cn)) {
                nodes[i]->getUniqueNodeList().push_back(cn);
                unl_count--;
            }
        }
    }

    // Create links
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

    // Print to json formatted text file
    std::cout << "Writing generated network to file: " << fileName << std::endl;
    file << "{" << std:: endl;
    file << "\t" << "\"numNodes\": " << numNodes << "," << std::endl;
    file << "\t" << "\"unlThresh\": " << UNL_MIN / 2 << "," << std::endl;
    file << "\t" << "\"nodes\": [" << std::endl;
    for (int i = 0; i < numNodes; i++) {
        file << "\t" << "\t" << nodes[i]->toJsonString();
        if (i != numNodes - 1) {
            file << ",";
        }
        file << std::endl;
    }
    file << "\t" << "]," << std::endl;
    file << "\t" << "\"links\": [ ";
    for (int i = 0; i < numNodes; i++) {
        int linkSize = nodes[i]->getLinks().size();
        for (int j = 0; j < linkSize; j++) {
            file << "{ " << "\"from\": " << nodes[i]->getNodeId() << ", ";
            file << "\"to\": " << nodes[i]->getLinks()[j].getToNodeId() << ", ";
            file << "\"latency\": " << nodes[i]->getLinks()[j].getTotalLatency() << " }";
            if (j != linkSize - 1) {
                file << ", ";
            }
        }
        if (i != numNodes - 1) {
            file << ", ";
        }
    }
    file << " ]" << std::endl;
    file << "}" << std::endl;
    file.close();

    return EXIT_SUCCESS;

}
