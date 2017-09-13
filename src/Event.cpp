#include "Event.h"

Event::Event() :
	messages() {
}

Message* Event::addMessage(const Message& message) {
    messages.push_back(message);
    return &*messages.rbegin();
}

const std::list<Message>& Event::getMessages() const {
    return messages;
}
