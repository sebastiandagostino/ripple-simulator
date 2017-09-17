#include <cassert>

#include "Network.h"

Network::Network() :
    masterTime(0), messages() {
	//messages.clear_deleted_key();
	//messages.set_deleted_key(-1);
}

int Network::getMasterTime() const {
    return masterTime;
}

void Network::setMasterTime(int masterTime) {
    this->masterTime = masterTime;
}

void Network::sendMessage(const Message& message, Link& link, int sendTime) {
    assert(message.getToNodeId() == link.getToNodeId());
    link.setSendTime(sendTime);
    link.setMessages(messages[link.getReceiveTime()].addMessage(message));
}

int Network::countMessages() const {
    return messages.size();
}

int Network::countMessagesOnTheWire() const {
    int mc = 0;
    for (const auto& event : messages) {
        mc += event.second.getMessages().size();
    }
    return mc;
}

EventMap& Network::getMessages() {
    return messages;
}
