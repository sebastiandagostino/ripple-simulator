#include "Link.h"

Link::Link(int toNodeId, int totalLatency) :
    toNodeId(toNodeId), totalLatency(totalLatency), sendTime(0), messages(0) {
}

int Link::getToNodeId() const {
    return toNodeId;
}

int Link::getReceiveTime() const {
    return sendTime + totalLatency;
}

int Link::getSendTime() const {
    return sendTime;
}

void Link::setSendTime(int sendTime) {
    this->sendTime = sendTime;
}

int Link::getTotalLatency() const {
    return totalLatency;
}

Message* Link::getMessages() {
    return messages;
}

void Link::setMessages(Message* messages) {
    this->messages = messages;
}
