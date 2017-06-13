#include "Link.h"

Link::Link(int t, int tl) :
		to_node(t), total_latency(tl), lm_send_time(0), lm_recv_time(0), lm(0) {
	;
}

