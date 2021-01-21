#ifndef BLEEP_ADDON_H
#define BLEEP_ADDON_H

// bleep_addon.h and cpp is for interfaces between Shadow and bleep-addon module.
// So, do not write functionality logic for each tasks in this code,
// just make submodule directories and separate them to other submodules.

#ifdef __cplusplus
extern "C"
{
#endif

/* bitcoin_mine_support related interface */
void bleep_addon_bitcoin_register_hash(const char hash[]);
int bleep_addon_bitcoin_check_hash(const char hash[]);

#ifdef __cplusplus
}
#endif

#endif
