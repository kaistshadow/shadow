//
// Created by csrc on 21. 4. 15..
//

#include "memory_sharing.h"

namespace memshare {
    std::unordered_map<std::type_index, memory_sharing_base*> global_mtbl_map;
    std::mutex global_mtbl_lock;
}