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

// Latencies in milliseconds
// E2C - End to core, the latency from a node to a nearby node
// C2C - Core to core, the additional latency when nodes are far

int main(int argc, char* argv[]) {

	std::string fileName;

	if (argc == 2) {
		fileName = argv[1];
	} else {
		std::cerr << "usage: sim input_file" << std::endl;
		return EXIT_FAILURE;
	}

	// Read parameters and network from json file

	std::cout << "Loading network from file: " << fileName << std::endl;

	std::ifstream file(fileName.c_str());
	json j;
	file >> j;

	if (j.find("numNodes") == j.end()) {
		std::cerr << "Value NUM_NODES not found. Exiting..." << std::endl;
		return EXIT_FAILURE;
	}
	int numNodes = j.find("numNodes").value().get<int>();
	std::cout << "Reading NUM_NODES = " << numNodes << std::endl;

	if (j.find("unlThresh") == j.end()) {
		std::cerr << "Value UNL_THRESH not found. Exiting..." << std::endl;
		return EXIT_FAILURE;
	}
	int unlThresh = j.find("unlThresh").value().get<int>();
	std::cout << "Reading UNL_THRESH = " << unlThresh << std::endl;

	// Create nodes

	if (j.find("nodes") == j.end()) {
		std::cerr << "Nodes not found. Exiting..." << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Creating nodes" << std::endl;

	Node* nodes[numNodes];

	json net(j.find("nodes").value());

	for (auto& element : net) {
		// NodeIds must be from 0 until numNodes - 1
		int i = element["nodeId"];
		int vote = element["vote"];
		int latency = element["latency"];
		nodes[i] = new Node(i, numNodes, latency);
		nodes[i]->getNodeStates()[i] = vote;
		nodes[i]->getNodeTimeStamps()[i] = 1;
		nodes[i]->setVote(vote);

		// Build our UNL
		json uniqueNodeList(element.find("uniqueNodeList").value());
		for (auto& unlNode : uniqueNodeList) {
			nodes[i]->getUniqueNodeList().push_back(unlNode);
		}
	}

	// Create links

	if (j.find("links") == j.end()) {
		std::cerr << "Links not found. Exiting..." << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Creating links" << std::endl;

	json links(j.find("links").value());

	for (auto& link : links) {
		int i = link["from"];
		int lt = link["to"];
		int latency = link["latency"];
		int ll = nodes[i]->getLatency() + nodes[lt]->getLatency() + latency;
		nodes[i]->getLinks().push_back(Link(lt, ll));
		nodes[lt]->getLinks().push_back(Link(i, ll));
	}

	// Trigger all nodes to make initial broadcasts of their own positions

	Network network;

	std::cout << "Creating initial messages" << std::endl;
	for (int i = 0; i < numNodes; i++) {
		for (Link& link : nodes[i]->getLinks()) {
			Message message(i, link.getToNodeId());
			message.insertData(i, nodes[i]->getNodeStates()[i]);
			network.sendMessage(message, link, 0);
		}

	}
	std::cout << "Created " << network.countMessages() << " events" << std::endl;

	// Run simulation
	std::cout << "      Time (ms)\t    Positive\t    Negative" << std::endl
			<< "      ---------\t    --------\t    --------" << std::endl;
	do {
		int nodesPositive = 0;
		int nodesNegative = 0;
		// Count nodes and check convergence
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
			std::cerr << "Fatal: Radio Silence. Exiting..." << std::endl;
			return EXIT_FAILURE;
		}

		if ((event->first / 100) > (network.getMasterTime() / 100)) {
			std::cout << "\t" << event->first << ";\t\t" << nodesPositive << ";\t\t"
					<< nodesNegative << std::endl;
		}
		network.setMasterTime(event->first);

		for (const Message& message : event->second.getMessages()) {
			if (message.hasEmptyData()) {
				// Message was never sent
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
	int nodesPositive = 0;
	int nodesNegative = 0;
	// Count nodes and check convergence
	for (int i = 0; i < numNodes; i++) {
		if (nodes[i]->getVote() > 0) {
			nodesPositive++;
		} else if (nodes[i]->getVote() < 0) {
			nodesNegative++;
		}
	}
	std::cout << "\t" << network.getMasterTime() << ";\t\t" << nodesPositive << ";\t\t"
			<< nodesNegative << std::endl;
	std::cout << "Consensus reached in " << network.getMasterTime()
			<< " ms with " << mc << " messages on the wire" << std::endl;

	// Output result
	long totalMsgsSent = 0;
	for (int i = 0; i < numNodes; i++) {
		totalMsgsSent += nodes[i]->getMessagesSent();
	}
	std::cout << "The average node sent " << totalMsgsSent / numNodes << " messages" << std::endl;

	// Clean up memory
	for (int i = 0; i < numNodes; i++) {
		delete nodes[i];
	}

	return EXIT_SUCCESS;

}
