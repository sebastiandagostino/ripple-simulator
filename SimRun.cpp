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
#include <jsoncpp/json/json.h>

#include "src/Event.h"
#include "src/Link.h"
#include "src/Message.h"
#include "src/Network.h"
#include "src/Node.h"
#include "src/NodeState.h"

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
    Json::Reader reader;
    Json::Value json;
    if (!reader.parse(file, json)) {
        std::cerr << "Error: " << reader.getFormattedErrorMessages() << std::endl;
        return EXIT_FAILURE;
    }

    int countParams = 0;
    for (auto const& id : json.getMemberNames()) {
    	if (id == "numNodes" || id == "unlThresh" || id == "nodes" || id == "links") {
    		countParams++;
    	}
    }
    if (countParams != 4) {
        std::cerr << "Json data: 'numNodes', 'unlThresh', 'nodes' or 'links' not found. Exiting..." << std::endl;
        return EXIT_FAILURE;
    }

    int numNodes = json["numNodes"].asInt();
    std::cout << "Reading NUM_NODES = " << numNodes << std::endl;
    int unlThresh = json["unlThresh"].asInt();
    std::cout << "Reading UNL_THRESH = " << unlThresh << std::endl;

    // Create nodes
    std::cout << "Creating nodes" << std::endl;
    Node* nodes[numNodes];

    const Json::Value& networkNodes = json["nodes"];
    for (const auto& element : networkNodes) {
        // NodeIds must be from 0 until numNodes - 1
        int nodeId = element["nodeId"].asInt();
        int vote = element["vote"].asInt();
        int latency = element["latency"].asInt();
        nodes[nodeId] = new Node(nodeId, numNodes, latency);
        nodes[nodeId]->getNodeStates()[nodeId] = vote;
        nodes[nodeId]->getNodeTimeStamps()[nodeId] = 1;
        nodes[nodeId]->setVote(vote);

        // Build our UNL
        const Json::Value& uniqueNodeList = element["uniqueNodeList"];
        for (const auto& unlNode : uniqueNodeList) {
            nodes[nodeId]->getUniqueNodeList().push_back(unlNode.asInt());
        }
    }

    // Create links
    std::cout << "Creating links" << std::endl;

    const Json::Value& networkLinks = json["links"];
    for (const auto& link : networkLinks) {
        int i = link["from"].asInt();
        int lt = link["to"].asInt();
        int latency = link["latency"].asInt();
        int ll = nodes[i]->getLatency() + nodes[lt]->getLatency() + latency;
        nodes[i]->getLinks().push_back(Link(lt, ll));
        nodes[lt]->getLinks().push_back(Link(i, ll));
    }

    Network network;

    // Trigger all nodes to make initial broadcasts of their own positions
    std::cout << "Creating initial messages" << std::endl;
    for (const auto& node : nodes) {
        for (auto& link : node->getLinks()) {
            Message message(node->getNodeId(), link.getToNodeId());
            message.insertData(node->getNodeId(), node->getNodeStates()[node->getNodeId()]);
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
        for (const auto& node : nodes) {
            if (node->getVote() > 0) {
                nodesPositive++;
            } else if (node->getVote() < 0) {
                nodesNegative++;
            }
        }
        if (nodesPositive > (numNodes * CONSENSUS_PERCENT / 100)) {
            break;
        }
        if (nodesNegative > (numNodes * CONSENSUS_PERCENT / 100)) {
            break;
        }

        if (network.getMessages().empty()) {
            std::cerr << "Fatal: Radio Silence. Exiting..." << std::endl;
            return EXIT_FAILURE;
        }

        EventMap::iterator event = network.getMessages().begin();
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
    } while (true);

    int nodesPositive = 0;
    int nodesNegative = 0;
    // Count nodes and check convergence
    for (const auto& node : nodes) {
        if (node->getVote() > 0) {
            nodesPositive++;
        } else if (node->getVote() < 0) {
            nodesNegative++;
        }
    }
    std::cout << "\t" << network.getMasterTime() << ";\t\t" << nodesPositive << ";\t\t" << nodesNegative << std::endl;
    std::cout << "Consensus reached in " << network.getMasterTime() << " ms with "
    		<< network.countMessagesOnTheWire() << " messages on the wire" << std::endl;

    // Output result
    long totalMsgsSent = 0;
    for (const auto& node : nodes) {
        totalMsgsSent += node->getMessagesSent();
    }
    std::cout << "The average node sent " << totalMsgsSent / numNodes << " messages" << std::endl;

    return EXIT_SUCCESS;

}
