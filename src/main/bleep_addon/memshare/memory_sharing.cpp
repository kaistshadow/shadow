//
// Created by csrc on 21. 4. 15..
//

#include "memory_sharing.h"

#include <typeindex>

void memshare::memory_sharing_unspecified::lock_tbl() {
    tbl_lock.lock();
}
void memshare::memory_sharing_unspecified::unlock_tbl() {
    tbl_lock.unlock();
}

std::unordered_map<std::type_index, memshare::memory_sharing_unspecified*> global_mtbl_map;
std::mutex global_mtbl_lock;

void try_register_memshare_table(void* type_idx_ref, void* mtbl) {
    std::type_index* type_idx_ptr = (std::type_index*)type_idx_ref;
    global_mtbl_lock.lock();
    memshare::memory_sharing_unspecified* p = (memshare::memory_sharing_unspecified*)mtbl;
    auto res = global_mtbl_map.insert({*type_idx_ptr, p});
    global_mtbl_lock.unlock();
    if (!res.second) {
        delete p;
    }
}
void memshare_try_share(void* type_idx_ref, void* sptr_ref) {
    std::type_index* type_idx_ptr = (std::type_index*)type_idx_ref;
    global_mtbl_lock.lock();
    auto it = global_mtbl_map.find(*type_idx_ptr);
    global_mtbl_lock.unlock();
    memshare::memory_sharing_unspecified* mtbl = it->second;
    mtbl->try_share(sptr_ref);
}
void* memshare_lookup(void* type_idx_ref, void* sptr_ref) {
    std::type_index* type_idx_ptr = (std::type_index*)type_idx_ref;
    global_mtbl_lock.lock();
    auto it = global_mtbl_map.find(*type_idx_ptr);
    global_mtbl_lock.unlock();
    memshare::memory_sharing_unspecified* mtbl = it->second;
    return mtbl->lookup(sptr_ref);
}
