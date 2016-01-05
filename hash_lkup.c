#include "hash_lkup.h"

#include <rte_hash.h>
#include <rte_eal.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_jhash.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct rte_hash *ht_hdl;

void hash_table_init(char *hash_file)
{
    ht_hdl = create_hash_table(hash_file);
}

int hash_table_lkup(const void *key)
{
    return rte_hash_lookup(ht_hdl, key);
}
