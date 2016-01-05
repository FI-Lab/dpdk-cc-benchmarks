#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>

#include <rte_eal.h>
#include <rte_mempool.h>
#include <rte_ring.h>
#include <rte_prefetch.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_branch_prediction.h>
#include <rte_common.h>

#include <rte_ip.h>
#include <rte_ether.h>

#include "fwd_util.h"
#include "pc.h"
#include "dpi.h"
#include "hash_lkup.h"
#include "pacnt.h"

//#define EXEC_HASH
//#define EXEC_PC
//#define EXEC_CRC
#define EXEC_DPI

//#define EXEC_MBUF_PA_CNT

#define NB_MBUF 131071
#define NB_RXD 128
#define NB_TXD 512
#define NB_LCACHE_SIZE 512
#define NB_BURST 32

#define NB_PORT_DFT 2
static uint32_t nb_port = NB_PORT_DFT;
static uint8_t is_stop = 0;

#define MAX_FILE_NAME_SZ 256
static char dpi_file[MAX_FILE_NAME_SZ];
static char pc_rule_file[MAX_FILE_NAME_SZ];
static char pc_fib_file[MAX_FILE_NAME_SZ];
static char hash_file[MAX_FILE_NAME_SZ];

struct rte_eth_conf port_conf = 
{
	.rxmode = 
	{
		.mq_mode = ETH_MQ_RX_RSS,
		.max_rx_pkt_len = ETHER_MAX_LEN,
	},
	.txmode = 
	{
		.mq_mode = ETH_MQ_TX_NONE,
	},
	.rx_adv_conf = 
	{
		.rss_conf = 
		{
			.rss_hf = ETH_RSS_IP,
		},
	},
};

struct port_list_t
{
	int portid;
	int queueid;
};

struct lcore_arg_t
{
	struct port_list_t pl[RTE_MAX_ETHPORTS];
	int pl_len;
	struct rte_mempool *mp;
	struct rte_mbuf *tx_buf[RTE_MAX_ETHPORTS][NB_BURST];
#ifdef EXEC_MBUF_PA_CNT
	struct rte_hash *pa_ht;
#endif
}lcore_args[RTE_MAX_LCORE];

static void usage()
{
	printf("fwd <EAL options> --"
           "-p <port number >=2 even>"
	       "-r <rule file neccessary>"
	       "-l <fib file neccessary> "
	       "-P <dpi file neccessary>"
	       "-H <hash file neccessary>\n");
}

static int parse_args(int argc, char **argv)
{
	int ch;
	int r = 0, l = 0, P = 0, H = 0;
	while((ch = getopt(argc, argv, "q:p:r:l:P:H:s")) != -1)
	{
		switch(ch)
		{
		case 'p':
			nb_port = atoi(optarg);
			printf("port number %u\n", nb_port);
			if(nb_port < 2 || (nb_port & 1))
			{
				usage();
				return -1;
			}
			break;
		case 'r':
			rte_memcpy(pc_rule_file, optarg, strlen(optarg) + 1);
			r = 1;
			break;
		case 'l':
			rte_memcpy(pc_fib_file, optarg, strlen(optarg) + 1);
			l = 1;
			break;
		case 'P':
			rte_memcpy(dpi_file, optarg, strlen(optarg) + 1);
			P = 1;
			break;
		case 'H':
			rte_memcpy(hash_file, optarg, strlen(optarg) + 1);
			H = 1;
			break;
		default:
			usage();
			return -1;
		}
	}
	if(!r || !l || !P || !H)
	{
		usage();
	}
	return 0;
}

static int setup_and_bond_ports(struct rte_mempool *mp)
{
	int portid, queueid;
	int ret;
	int pl_idx;
	int nb_queue;
	
	nb_queue = rte_lcore_count();

	nb_port = rte_eth_dev_count();

    memset(lcore_args, 0, sizeof(struct lcore_arg_t) * RTE_MAX_LCORE);

	for(portid = 0; portid < nb_port; portid++)
	{
		ret = rte_eth_dev_configure(portid, nb_queue, nb_queue, &port_conf);
		if(unlikely(ret < 0))
		{
			rte_exit(EINVAL, "port %d configure failed!\n", portid);
		}
		for(queueid = 0; queueid < nb_queue; queueid++)
		{
			ret = rte_eth_rx_queue_setup(portid, queueid, NB_RXD, rte_socket_id(), NULL, mp);
			if(unlikely(ret < 0))
			{
				rte_exit(EINVAL, "port %d rx queue %d setup failed!\n", portid, queueid);
			}
			ret = rte_eth_tx_queue_setup(portid, queueid, NB_TXD, rte_socket_id(), NULL);
			if(unlikely(ret < 0))
			{
				rte_exit(EINVAL, "port %d tx queue %d setup failed!\n", portid, queueid);
			}
            pl_idx = lcore_args[queueid].pl_len;
			lcore_args[queueid].pl[pl_idx].portid = portid;
            lcore_args[queueid].pl[pl_idx].queueid = queueid;
            lcore_args[queueid].mp = mp;
            lcore_args[queueid].pl_len = pl_idx + 1;
		}
		ret = rte_eth_dev_start(portid);
		if(unlikely(ret < 0))
		{
			rte_exit(EINVAL, "port %d start failed!\n", portid);
		}
		rte_eth_promiscuous_enable(portid);
	}
	return 0;
}

static void process_pkts(struct rte_mbuf *buf[], int n)
{
	int i;
	uint8_t *pkt;
	int ret;
	uint32_t ft[5];
	unsigned char *payload;
	int len;
	for(i = 0; i < n; i++)
	{   
#ifdef EXEC_MBUF_PA_CNT
		uint32_t lcoreid = rte_lcore_id();
		uint32_t *count;
		struct rte_hash *h = lcore_args[lcoreid].pa_ht;
		if(rte_hash_lookup_data(h, (const void *)&(buf[i]->buf_physaddr), (void**)&count) >= 0)
		{
			*count = *count + 1;
		}
		else
		{
			if(pacnt_hash_add(h, (const void *)&(buf[i]->buf_physaddr), 1) < 0)
			{
				rte_exit(EINVAL, "pacnt hash add failed in lcore %d\n", lcoreid);
			}
		}
#endif
#if defined(EXEC_PC) || defined(EXEC_HASH)
		parse_packet_to_tuple(buf[i], ft);
#ifdef EXEC_PC
		ret = packet_classifier_search(ft);       
		if(ret < 0)
		{
			fprintf(stderr, "packet classifing failed!\n");
		}
#else
		ret = hash_table_lkup((void*)ft);
#endif
#endif
#ifdef EXEC_CRC
		calc_chk_sum(buf[i]);
#endif
#ifdef EXEC_DPI
		ret = get_payload(buf[i], &payload, &len);
		if(ret < 0)
		{
			fprintf(stderr, "packet get payload failed!\n");
			continue;
		}
		ret = dpi_engine_exec(payload, len);
#endif
	}
}

static int fwd_loop(__attribute__((__unused__))void *arg)
{
	int lcoreid, portid, queueid;
	int nb_rx, nb_tx;
	int pl_len, i;
	lcoreid = rte_lcore_id();
	pl_len = lcore_args[lcoreid].pl_len;
	printf("lcore %d from socket %d\n", rte_lcore_id(), rte_socket_id());
    for(i = 0; i < pl_len; i++)
	{
		portid = lcore_args[lcoreid].pl[i].portid;
		queueid = lcore_args[lcoreid].pl[i].queueid;
		printf("lcore %d enter loop port %d queue %d\n", lcoreid, portid, queueid);
	}
#ifdef EXEC_MBUF_PA_CNT
	lcore_args[lcoreid].pa_ht = pacnt_hash_create(NB_MBUF << 1);
	if(lcore_args[lcoreid].pa_ht == NULL)
	{
		rte_exit(EINVAL, "pacnt hash created failed on lcore %d\n", lcoreid);
	}
#endif
	while(!is_stop)
	{
		for(i = 0; i < pl_len; i++)
		{
			portid = lcore_args[lcoreid].pl[i].portid;
			queueid = lcore_args[lcoreid].pl[i].queueid;
			nb_rx = rte_eth_rx_burst(portid, queueid, lcore_args[lcoreid].tx_buf[i], NB_BURST);
			process_pkts(lcore_args[lcoreid].tx_buf[i], nb_rx);
			nb_tx = rte_eth_tx_burst(portid^1, queueid, lcore_args[lcoreid].tx_buf[i], nb_rx);
			/*if(nb_rx >=0 && nb_tx >= 0)
			  {
			  printf("lcore %d rx %d tx %d\n", lcoreid, nb_rx, nb_tx);
			  }*/
			while(nb_tx < nb_rx)
			{
				rte_pktmbuf_free(lcore_args[lcoreid].tx_buf[i][nb_tx++]);
			}
		}
	}
	printf("lcore %d finished!\n", rte_lcore_id());
}

#ifdef EXEC_MBUF_PA_CNT
static void sig_hdl(int sig)
{
	is_stop = 1;
	printf("sigint executed by lcore %d\n", rte_lcore_id());
}
static void write_pa()
{
	struct rte_hash *total, *h;
	uint32_t lcoreid, nb_lcore;
	phys_addr_t *key;
	uint32_t *data, *newdata;
	uint32_t next;
	FILE *fp;
	char file_name[MAX_FILE_NAME_SZ];
	printf("start writing paddr!\n");
	nb_lcore = rte_lcore_count();
	total = pacnt_hash_create(NB_MBUF << 1);
	if(total == NULL)
	{
		rte_exit(EINVAL, "total pacnt hash create failed!\n");
	}
	for(lcoreid = 0; lcoreid < nb_lcore; lcoreid++)
	{
		h = lcore_args[lcoreid].pa_ht;
		next = 0;
		if(h == NULL)
		{   
			rte_exit(EINVAL, "lcore %d paddr hash == NULL!\n", lcoreid);
		}
		snprintf(file_name, MAX_FILE_NAME_SZ, "paddr/paddr_lcore_%u", lcoreid);
		fp = fopen(file_name, "w");
		while(rte_hash_iterate(h, (const void**)&key, (void**)&data, &next) >= 0)
		{
			if(rte_hash_lookup_data(total, (const void*)key, (void**)&newdata) >= 0)
			{
				*newdata += *data;
			}
			else
			{
				if(pacnt_hash_add(total, (const void*)key, *data) < 0)
				{
					rte_exit(EINVAL, "total pacnt hash add data failed!\n");
				}
			}
			fprintf(fp, "paddr: %llx, count: %llu\n", (unsigned long long)*key, (unsigned long long)*data);
		}
		fflush(fp);
		fclose(fp);
		//rte_hash_free(h);
	}

	fp = fopen("paddr/paddr_total", "w");
	next = 0;
	while(rte_hash_iterate(total, (const void**)&key, (void**)&data, &next) >= 0)
	{
		fprintf(fp, "paddr: %llx, count: %llu\n", (unsigned long long)*key, (unsigned long long)*data);
	}
	fflush(fp);
	fclose(fp);
	//rte_hash_free(total);
	printf("count finished!\n");
}
#endif

int main(int argc, char **argv)
{
	int ret;
	ret = rte_eal_init(argc, argv);
	if(unlikely(ret < 0))
	{
		rte_exit(EINVAL, "EAL init failed!\n");
	}
	argc -= ret;
	argv += ret;
	ret = parse_args(argc, argv);
	if(unlikely(ret < 0))
	{
		rte_exit(EINVAL, "parse args failed!\n");
	}
#ifdef EXEC_MBUF_PA_CNT
	signal(SIGUSR1, sig_hdl);
#endif
	packet_classifier_init(pc_rule_file, pc_fib_file);
	dpi_engine_init(dpi_file);
	hash_table_init(hash_file);

	struct rte_mempool *mp;
	mp = rte_pktmbuf_pool_create("Lucky", NB_MBUF, NB_LCACHE_SIZE, 0, RTE_MBUF_DEFAULT_DATAROOM, rte_socket_id());
	if(unlikely(mp == NULL))
	{
		rte_exit(EINVAL, "create mempool failed!\n");
	}
	ret = setup_and_bond_ports(mp);

	ret = rte_eal_mp_remote_launch(fwd_loop, NULL, CALL_MASTER);
	if(unlikely(ret < 0))
	{
		rte_exit(EINVAL, "mp launch failed!\n");
	}
#ifdef EXEC_MBUF_PA_CNT
	write_pa();
#endif
	return 0;
}
