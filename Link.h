#ifndef LINK_H_
#define LINK_H_

#include "Message.h"

/**
 * A connection from one node to another
 */
class Link {

public:

    int to_node;

    int total_latency;

    int lm_send_time, lm_recv_time;

    Message* lm;

    Link(int t, int tl);

};

#endif /* LINK_H_ */
