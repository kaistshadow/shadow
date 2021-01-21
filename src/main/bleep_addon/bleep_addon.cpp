#include "bleep_addon.h"

// bleep_addon.h and cpp is for interfaces between Shadow and bleep-addon module.
// So, do not write functionality logic for each tasks in this code,
// just make submodule directories and separate them to other submodules.

#include "bitcoin_mine_support/coinflip_validation.h"

extern "C"
{

void bleep_addon_bitcoin_register_hash(const char hash[]) {
    return bitcoin_mine_support().register_hash(hash);
}
int bleep_addon_bitcoin_check_hash(const char hash[]) {
    return bitcoin_mine_support().check_hash(hash);
}


}