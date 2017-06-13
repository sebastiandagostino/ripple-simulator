#include <cassert>

#include "Network.h"

Network::Network() :
		master_time(0) {
	;
}

void Network::sendMessage(const Message& message, Link& link, int send_time) {
	assert(message.to_node == link.to_node);
	link.lm_send_time = send_time;
	link.lm_recv_time = send_time + link.total_latency;
	link.lm = messages[link.lm_recv_time].addMessage(message);
}
