//------------------------------------------------------------------------------
/*
 This file is part of consensus-sim
 Copyright (c) 2013, Ripple Labs Inc.

 Permission to use, copy, modify, and/or distribute this software for any
 purpose  with  or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
 MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
//==============================================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <random>

#include "src/Event.h"
#include "src/Link.h"
#include "src/Message.h"
#include "src/Network.h"
#include "src/Node.h"
#include "src/NodeState.h"

// JSON for modern C++
// https://github.com/nlohmann/json
#include "lib/json.hpp"
using nlohmann::json;

#define CONSENSUS_PERCENT       80

#define DEFAULT_FILE 			"network.json"

// Latencies in milliseconds
// E2C - End to core, the latency from a node to a nearby node
// C2C - Core to core, the additional latency when nodes are far

int main(int argc, char* argv[]) {

	std::string fileName = DEFAULT_FILE;

	if (argc == 2) {
		fileName = argv[1];
	}

	// read parameters and network from json file

	std::cout << "Loading network from file: " << fileName << std::endl;

	std::ifstream file(fileName.c_str());

	std::string jsonString = static_cast<std::stringstream const&>(std::stringstream() << file.rdbuf()).str();

	auto j = json::parse(jsonString);

	if (j.find("numNodes") == j.end()) {
	  std::cerr << "Value " << "NUM_NODES" << " not found. Exiting..." << std::endl;
	  return -1;
	}
	int numNodes = j.find("numNodes").value().get<int>();
	std::cout << "Reading NUM_NODES = " << numNodes << std::endl;
	
	if (j.find("unlMin") == j.end()) {
	  std::cerr << "Value " << "UNL_MIN" << " not found. Exiting..." << std::endl;
	  return -1;
	}
	int unlMin = j.find("unlMin").value().get<int>();
	std::cout << "Reading UNL_MIN = " << unlMin << std::endl;
	int unlThresh = unlMin / 2;
	
	if (j.find("unlMax") == j.end()) {
	  std::cerr << "Value " << "UNL_MAX" << " not found. Exiting..." << std::endl;
	  return -1;
	}
	int unlMax = j.find("unlMax").value().get<int>();
	std::cout << "Reading UNL_MAX = " << unlMax << std::endl;
	
	if (j.find("numOutboundLinks") == j.end()) {
	  std::cerr << "Value " << "NUM_OUTBOUND_LINKS" << " not found. Exiting..." << std::endl;
	  return -1;
	}
	int numOutboundLinks = j.find("numOutboundLinks").value().get<int>();
	std::cout << "Reading NUM_OUTBOUND_LINKS = " << numOutboundLinks << std::endl;
	
	if (j.find("minLatencyE2C") == j.end()) {
	  std::cerr << "Value " << "MIN_E2C_LATENCY" << " not found. Exiting..." << std::endl;
	  return -1;
	}
	int minE2C = j.find("minLatencyE2C").value().get<int>();
	std::cout << "Reading MIN_E2C_LATENCY = " << minE2C << std::endl;
	
	if (j.find("maxLatencyE2C") == j.end()) {
	  std::cerr << "Value " << "MAX_E2C_LATENCY" << " not found. Exiting..." << std::endl;
	  return -1;
	}
	int maxE2C = j.find("maxLatencyE2C").value().get<int>();
	std::cout << "Reading MAX_E2C_LATENCY = " << maxE2C << std::endl;
	
	if (j.find("minLatencyC2C") == j.end()) {
	  std::cerr << "Value " << "MIN_C2C_LATENCY" << " not found. Exiting..." << std::endl;
	  return -1;
	}
	int minC2C = j.find("minLatencyC2C").value().get<int>();
	std::cout << "Reading MIN_C2C_LATENCY = " << minC2C << std::endl;
	
	if (j.find("maxLatencyC2C") == j.end()) {
	  std::cerr << "Value " << "MAX_C2C_LATENCY" << " not found. Exiting..." << std::endl;
	  return -1;
	}
	int maxC2C = j.find("maxLatencyC2C").value().get<int>();
	std::cout << "Reading MAX_C2C_LATENCY = " << maxC2C << std::endl;
		
	// This will produce the same results each time
	std::mt19937 gen; // Standard mersenne_twister_engine
	std::uniform_int_distribution<> r_e2c(minE2C, maxE2C);
	std::uniform_int_distribution<> r_c2c(minC2C, maxC2C);
	std::uniform_int_distribution<> r_unl(unlMin, unlMax);
	std::uniform_int_distribution<> r_node(0, numNodes - 1);

	Node* nodes[numNodes];

	// create nodes
	std::cerr << "Creating nodes" << std::endl;
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
	std::cerr << "Creating links" << std::endl;
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

	Network network;

	// trigger all nodes to make initial broadcasts of their own positions
	std::cerr << "Creating initial messages" << std::endl;
	for (int i = 0; i < numNodes; i++) {
		for (Link& link : nodes[i]->getLinks()) {
			Message message(i, link.getToNodeId());
			message.insertData(i, nodes[i]->getNodeStates()[i]);
			network.sendMessage(message, link, 0);
		}
	}
	std::cerr << "Created " << network.countMessages() << " events" << std::endl;

	// run simulation
	do {
		// count nodes and check convergence
		int nodesPositive = 0;
		int nodesNegative = 0;
		for (int i = 0; i < numNodes; i++) {
			if (nodes[i]->getVote() > 0) {
				nodesPositive++;
			} else if (nodes[i]->getVote() < 0) {
				nodesNegative++;
			}
		}
		if (nodesPositive > (numNodes * CONSENSUS_PERCENT / 100)) {
			break;
		}
		if (nodesNegative > (numNodes * CONSENSUS_PERCENT / 100)) {
			break;
		}

		std::map<int, Event>::iterator event = network.getMessages().begin();
		if (event == network.getMessages().end()) {
			std::cerr << "Fatal: Radio Silence" << std::endl;
			return 0;
		}

		if ((event->first / 100) > (network.getMasterTime() / 100)) {
			std::cerr << "Time: " << event->first << " ms  " << nodesPositive << "/" << nodesNegative << std::endl;
		}
		network.setMasterTime(event->first);

		for (const Message& message : event->second.getMessages()) {
			if (message.hasEmptyData()) {
				// message was never sent
				nodes[message.getFromNodeId()]->decreaseMessagesSent();
			} else {
				nodes[message.getToNodeId()]->receiveMessage(message, network, unlThresh);
			}
		}

		network.getMessages().erase(event);
	} while (1);

	int mc = 0;
	std::map<int, Event>::iterator it;
	for (it = network.getMessages().begin(); it != network.getMessages().end(); it++) {
		mc += it->second.getMessages().size();
	}
	std::cerr << "Consensus reached in " << network.getMasterTime() << " ms with "
			<< mc << " messages on the wire" << std::endl;

	// output result
	long totalMsgsSent = 0;
	for (int i = 0; i < numNodes; i++) {
		totalMsgsSent += nodes[i]->getMessagesSent();
	}
	std::cerr << "The average node sent " << totalMsgsSent / numNodes << " messages" << std::endl;

	return EXIT_SUCCESS;

}
