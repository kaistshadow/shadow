//
// Created by csrc on 21. 1. 5..
//

#ifndef STORAGE_SHARING_MODULE_SHARING_TRACKER_H
#define STORAGE_SHARING_MODULE_SHARING_TRACKER_H

#include "../data_unit/data_segment.h"
#include "../datatypes/datatype.h"

#include <mutex>
#include <unordered_map>
#include <unordered_set>

class hashfn {
public:
    std::size_t operator()(data_segment* obj) const;
};
class equalfn {
public:
    bool operator()(data_segment* lhs, data_segment* rhs) const;
};

class sharing_tracker {
    std::unordered_set<data_segment*, hashfn, equalfn> tracker;
    std::mutex sharing_tracker_mutex;
public:
    std::unordered_set<data_segment*, hashfn, equalfn>::iterator find(data_segment* s);
    std::unordered_set<data_segment*, hashfn, equalfn>::iterator end();
    void insert(sharing_unit* s);
    void remove(sharing_unit* s);
    void lock();
    void unlock();
};

void init_sharing_tracker_list();
sharing_tracker* get_sharing_tracker(int tracker_id);
#endif //STORAGE_SHARING_MODULE_SHARING_TRACKER_H
