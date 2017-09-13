#ifndef NETWORK_H_
#define NETWORK_H_

#include <map>

#include "Event.h"
#include "Link.h"
#include "Message.h"

typedef std::map<int, Event> EventMap;

class Network {

private:

    int masterTime;

    EventMap messages;

public:

    Network();

    int getMasterTime() const;

    void setMasterTime(int masterTime);

    void sendMessage(const Message& message, Link& link, int sendTime);

    int countMessages() const;

    int countMessagesOnTheWire() const;

    EventMap& getMessages();

};

#endif /* NETWORK_H_ */
