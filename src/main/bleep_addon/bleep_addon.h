#ifndef BLEEP_ADDON_H
#define BLEEP_ADDON_H

#include "memshare/memory_sharing.h"

#ifdef __cplusplus
extern "C"
{
#endif

void shadow_bitcoin_register_hash(const char hash[], int reindex);
int shadow_bitcoin_check_hash(const char hash[]);
void shadow_bitcoin_load_hash();

#ifdef __cplusplus
}
#endif


#endif
