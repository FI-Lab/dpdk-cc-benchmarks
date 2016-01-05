#include "pacnt.h"
#include <rte_eal.h>
#include <rte_common.h>
#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_memory.h>
#include <rte_mbuf.h>
#include <rte_malloc.h>
#include <rte_lcore.h>

struct rte_hash* pacnt_hash_create(uint32_t entries)
{
    struct rte_hash *h;
    struct rte_hash_parameters param;
    char name[64];
    snprintf(name, 64, "addr_hash_%d", rte_lcore_id());
    memset(&param, 0, sizeof(param));
    param.name = name;
    param.entries = entries;
    param.key_len = sizeof(phys_addr_t);
    param.hash_func = rte_jhash;
    param.hash_func_init_val = 1234;
    param.socket_id = rte_socket_id();
    h = rte_hash_create(&param);
    return h;
}

int pacnt_hash_add(const struct rte_hash *h, const void *key, uint32_t count)
{
    uint32_t *c = (uint32_t*)rte_malloc(NULL, sizeof(uint32_t), 0);
    if(c == NULL)
    {
        rte_exit(EINVAL, "hash add malloc failed!\n");
    }
    *c = count;
    return rte_hash_add_key_data(h, key, (void*)c);
}
