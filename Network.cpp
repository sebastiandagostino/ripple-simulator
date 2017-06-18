#include <cassert>

#include "Network.h"

Network::Network() :
	master_time(0) {
}

void Network::sendMessage(const Message& message, Link& link, int send_time) {
	assert(message.getToNodeId() == link.getToNodeId());
	link.setSendTime(send_time);
	link.setMessages(messages[link.getReceiveTime()].addMessage(message));
}
