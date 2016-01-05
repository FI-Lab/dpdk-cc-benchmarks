#ifndef PACNT_H
#define PACNT_H

#include <rte_hash.h>
#include <rte_mbuf.h>

struct rte_hash* pacnt_hash_create(uint32_t entries);

int pacnt_hash_add(const struct rte_hash *h, const void *key, uint32_t count);

#endif
