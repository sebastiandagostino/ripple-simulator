#ifndef EVENT_H_
#define EVENT_H_

#include <list>

#include "Message.h"

/**
 * One or more messages that are received at a particular time
 */
class Event {

private:

	std::list<Message> messages;

public:

	Message* addMessage(const Message& m);

	const std::list<Message>& getMessages() const;

};

#endif /* EVENT_H_ */
