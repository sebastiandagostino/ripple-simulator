#include "Event.h"

Message* Event::addMessage(const Message& m) {
    messages.push_back(m);
    return &*messages.rbegin();
}

const std::list<Message>& Event::getMessages() const {
    return messages;
}
