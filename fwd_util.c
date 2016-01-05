#include "fwd_util.h"
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_ether.h>
#include <rte_udp.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void parse_packet_to_tuple(struct rte_mbuf *m, uint32_t *ft)
{
	struct ipv4_hdr *ip;
	struct udp_hdr *udp;
    void *packet;
    packet = rte_pktmbuf_mtod(m, void*);
	ip = (struct ipv4_hdr*)(packet + sizeof(struct ether_hdr));
	udp = (struct udp_hdr*)(packet + sizeof(struct ether_hdr) 
		+ sizeof(struct ipv4_hdr));
	ft[0] = ntohl(ip->src_addr);
	ft[1] = ntohl(ip->dst_addr);
	ft[2] = ntohs(udp->src_port);
	ft[3] = ntohs(udp->dst_port);
	ft[4] = ip->next_proto_id;
	
#if 0
	static int two = 0;
	if(two < 2)
	{
		two++;
		printf("saddr %u.%u.%u.%u\n",0xff&(ft[0]>>24),0xff&(ft[0]>>16),0xff&(ft[0]>>8),0xff&(ft[0]));
		printf("daddr %u.%u.%u.%u\n",0xff&(ft[1]>>24),0xff&(ft[1]>>16),0xff&(ft[1]>>8),0xff&(ft[1]));
		printf("sport %u\n",ft[2]);
		printf("dport %u\n",ft[3]);
		printf("proto %u\n",ft[4]);
	}
#endif
}

void calc_chk_sum(struct rte_mbuf *m)
{
	uint8_t sum;
	uint32_t i;
	struct ipv4_hdr *ip;
	uint32_t len;
    uint8_t *pkt;
    pkt = rte_pktmbuf_mtod(m, uint8_t*);
	ip = (struct ipv4_hdr*)(pkt + sizeof(struct ether_hdr));
	len = ntohs(ip->total_length) + sizeof(struct ether_hdr);
	for(i = 0, sum = 0; i < len; i++)
	{
		sum += pkt[i];
	}
	pkt[i-1] = sum;
}

int get_payload(struct rte_mbuf *m, unsigned char **payload, int *len)
{
    struct ipv4_hdr *ip;
    struct tcp_hdr *tcp;
    int payload_offset;
    ip = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr*, sizeof(struct ether_hdr));
    if(ip == NULL)
    {
    	fprintf(stderr, "recv non-ip packet!\n");
        return -1;
    }
    payload_offset = (ip->version_ihl & 0xf) << 2;
    if(ip->next_proto_id == 6)//TCP
    {
        tcp = (struct tcp_hdr*)((uint8_t*)ip + payload_offset);
        //printf("%d\n", (tcp->hl & 0xf) << 2);
        payload_offset += ((tcp->data_off >> 4) & 0xf) << 2;
        *len = ntohs(ip->total_length) - payload_offset;
        *payload = (unsigned char*)ip + payload_offset;
        //printf("payload %s\n", (char*)*payload);
        return 0;
    }
    else if(ip->next_proto_id == 17)
    {
    	payload_offset += 8;
        *len = ntohs(ip->total_length) - payload_offset;
        *payload = (unsigned char*)ip + payload_offset;
        return 0;
    }
    fprintf(stderr, "recv non-tcp/udp packet!\n");
    return -1;
}
