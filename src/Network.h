#ifndef NETWORK_H_
#define NETWORK_H_

#include <map>

#include "Event.h"
#include "Link.h"
#include "Message.h"

class Network {

private:

	int masterTime;

	std::map<int, Event> messages;

public:

	Network();

	int getMasterTime() const;

	void setMasterTime(int masterTime);

	void sendMessage(const Message& message, Link& link, int sendTime);

	int countMessages() const;

	std::map<int, Event>& getMessages();

};

#endif /* NETWORK_H_ */
