#ifndef LINK_H_
#define LINK_H_

#include "Message.h"

/**
 * A connection from one node to another
 */
class Link {

private:

    int toNodeId;

    int totalLatency;

    int sendTime;

    Message* messages;

public:

    Link(int toNodeId, int totalLatency);

    int getToNodeId() const;

    int getReceiveTime() const;

    int getSendTime() const;

    void setSendTime(int sendTime);

    int getTotalLatency() const;

    Message* getMessages();

    void setMessages(Message* messages);

};

#endif /* LINK_H_ */
