#ifndef BLEEP_BITCOIN_MINE_SUPPORT_H
#define BLEEP_BITCOIN_MINE_SUPPORT_H

#include <unordered_set>
#include <mutex>
#include <string>

class bitcoin_mine_support {
public:
	static void register_hash(const char hash[]);
	static int check_hash(const char hash[]);
};

#endif