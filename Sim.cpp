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
#include <random>

#include "Event.h"
#include "Link.h"
#include "Message.h"
#include "Network.h"
#include "Node.h"
#include "NodeState.h"

/*
#define LEDGER_CONVERGE          4
#define LEDGER_FORCE_CONVERGE    7
#define AV_MIN_CONSENSUS        50
#define AV_AVG_CONSENSUS        60
#define AV_MAX_CONSENSUS        70
*/

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

int nodes_positive = 0;
int nodes_negative = 0;

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
	for (int i = 0; i < NUM_NODES; ++i) {
		nodes[i] = new Node(i, NUM_NODES);
		nodes[i]->e2c_latency = r_e2c(gen);

		// our own position starts as 50/50 split
		if (i % 2) {
			nodes[i]->knowledge[i] = 1;
			nodes[i]->nts[i] = 1;
			nodes_positive++;
		} else {
			nodes[i]->knowledge[i] = -1;
			nodes[i]->nts[i] = 1;
			nodes_negative++;
		}

		// Build our UNL
		int unl_count = r_unl(gen);
		while (unl_count > 0) {
			int cn = r_node(gen);
			if ((cn != i) && !nodes[i]->isOnUNL(cn)) {
				nodes[i]->unl.push_back(cn);
				unl_count--;
			}
		}
	}

	// create links
	std::cerr << "Creating links" << std::endl;
	for (int i = 0; i < NUM_NODES; ++i) {
		int links = NUM_OUTBOUND_LINKS;
		while (links > 0) {
			int lt = r_node(gen);
			if ((lt != i) && !nodes[i]->hasLinkTo(lt)) {
				int ll = nodes[i]->e2c_latency + nodes[lt]->e2c_latency + r_c2c(gen);
				nodes[i]->links.push_back(Link(lt, ll));
				nodes[lt]->links.push_back(Link(i, ll));
				links--;
			}
		}
	}

	Network network;

	// trigger all nodes to make initial broadcasts of their own positions
	std::cerr << "Creating initial messages" << std::endl;
	for (int i = 0; i < NUM_NODES; ++i) {
		for (Link& l : nodes[i]->links) {
			Message m(i, l.getToNodeId());
			m.data.insert(std::make_pair(i, NodeState(i, 1, nodes[i]->knowledge[i])));
			network.sendMessage(m, l, 0);
		}
	}
	std::cerr << "Created " << network.messages.size() << " events" << std::endl;

	// run simulation
	do {
		if (nodes_positive > (NUM_NODES * CONSENSUS_PERCENT / 100)) {
			break;
		}
		if (nodes_negative > (NUM_NODES * CONSENSUS_PERCENT / 100)) {
			break;
		}

		std::map<int, Event>::iterator ev = network.messages.begin();
		if (ev == network.messages.end()) {
			std::cerr << "Fatal: Radio Silence" << std::endl;
			return 0;
		}

		if ((ev->first / 100) > (network.master_time / 100)) {
			std::cerr << "Time: " << ev->first << " ms  " << nodes_positive << "/" << nodes_negative << std::endl;
		}
		network.master_time = ev->first;

		for (const Message& m : ev->second.getMessages()) {
			if (m.data.empty()) {
				// message was never sent
				--nodes[m.from_node]->messages_sent;
			} else {
				nodes[m.to_node]->receiveMessage(m, network);
			}
		}

		network.messages.erase(ev);
	} while (1);

	int mc = 0;
	std::map<int, Event>::iterator it;
	for (it = network.messages.begin(); it != network.messages.end(); ++it) {
		mc += it->second.getMessages().size();
	}
	std::cerr << "Consensus reached in " << network.master_time << " ms with "
			<< mc << " messages on the wire" << std::endl;

	// output results
	long total_messages_sent = 0;
	for (int i = 0; i < NUM_NODES; i++) {
		total_messages_sent += nodes[i]->messages_sent;
	}
	std::cerr << "The average node sent " << total_messages_sent / NUM_NODES
			<< " messages" << std::endl;

}
