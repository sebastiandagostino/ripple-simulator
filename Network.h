#ifndef NETWORK_H_
#define NETWORK_H_

#include <map>

#include "Event.h"
#include "Link.h"
#include "Message.h"

class Network {

public:

	int master_time;

	std::map<int, Event> messages;

	Network();

	void sendMessage(const Message& message, Link& link, int send_time);

};

#endif /* NETWORK_H_ */
