//
// Created by csrc on 21. 1. 5..
//

#ifndef STORAGE_SHARING_MODULE_SHARING_TRACKER_H
#define STORAGE_SHARING_MODULE_SHARING_TRACKER_H

#include "../data_unit/data_segment.h"
#include "../datatypes/datatype.h"

#include <mutex>
#include <unordered_map>

class key {
public:
    data_segment* segment;
    key(data_segment* segment) {
        this->segment = segment;
    }
    virtual bool operator==(const key &other) const;
};
class hashfn {
public:
    std::size_t operator()(const key& k) const {
        return k.segment->hash();
    }
};

class sharing_tracker {
    std::unordered_map<key, sharing_unit*, hashfn> tracker;
    std::mutex sharing_tracker_mutex;
    public:
    std::unordered_map<key, sharing_unit*, hashfn>::iterator find(data_segment* s);
    std::unordered_map<key, sharing_unit*, hashfn>::iterator end();
    void insert(sharing_unit* s);
    void remove(sharing_unit* s);
    void lock();
    void unlock();
};

void init_sharing_tracker_list();
sharing_tracker* get_sharing_tracker(int tracker_id);
#endif //STORAGE_SHARING_MODULE_SHARING_TRACKER_H
