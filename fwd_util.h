#ifndef FWD_UTIL
#define FWD_UTIL

#include <rte_mbuf.h>

void parse_packet_to_tuple(struct rte_mbuf *m, uint32_t *ft);

void calc_chk_sum(struct rte_mbuf *m);

int get_payload(struct rte_mbuf *m, unsigned char **payload, int *len);

#endif
