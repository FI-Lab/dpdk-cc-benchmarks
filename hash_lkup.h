#ifndef HASH_LOOK_UP_H
#define HASH_LOOK_UP_H


#include <rte_hash.h>

#include <rte_hash.h>
#include <rte_eal.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_jhash.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define MAX_HASH_ENTRIES 1000000
#define FIVE_TUPLE_LEN (sizeof(uint32_t) * 5)

void hash_table_init(char *hash_file);
int hash_table_lkup(const void *key);

static struct rte_hash* create_hash_table(char *hash_file)
{
    struct rte_hash *hdl;
    struct rte_hash_parameters param;
    memset(&param, 0, sizeof(param));
    param.name = "L2FWD";
    param.entries = MAX_HASH_ENTRIES;
    param.key_len = FIVE_TUPLE_LEN;
    param.hash_func = rte_jhash;
    param.hash_func_init_val = 0x1234;
    hdl = rte_hash_create(&param);

    if(hdl == NULL)
    {
        rte_exit(EINVAL, "create hash table failed!\n");
    }

    uint32_t i;
    uint32_t ft[5];
    int ret;
    FILE *fp = fopen(hash_file, "r");
    if(fp == NULL)
    {
        rte_exit(EINVAL, "hash file not exist!\n");
    }
    i = 0;
    while(fscanf(fp, "%u%u%u%u%u", &ft[0], &ft[1], &ft[2], &ft[3], &ft[4]) != EOF)
    {
        ret = rte_hash_add_key(hdl, (void*)ft);
        if(ret == -EINVAL)
        {
            rte_exit(EINVAL, "add key %u failure!\n", i);
        }
        i++;
    }
    printf("load %u hash entries!\n", i);
    fclose(fp);
    return hdl;
}
#endif
