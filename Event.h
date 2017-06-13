#ifndef EVENT_H_
#define EVENT_H_

#include <list>

#include "Message.h"

/**
 * One or more messages that are received at a particular time
 */
class Event {

public:

	std::list<Message> messages;

	Message* addMessage(const Message& m);

};

#endif /* EVENT_H_ */
