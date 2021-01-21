//
// Created by csrc on 21. 1. 5..
//

#ifndef STORAGE_SHARING_MODULE_SHARING_TRACKER_H
#define STORAGE_SHARING_MODULE_SHARING_TRACKER_H

#include "../data_unit/data_segment.h"
#include "../datatypes/datatype.h"

#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include <cstdio>

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
    public:
    std::unordered_map<key, sharing_unit*, hashfn>::iterator find(data_segment* s) {
        key k(s);
        return tracker.find(k);
    }
    std::unordered_map<key, sharing_unit*, hashfn>::iterator end() {
        return tracker.end();
    }
    void insert(sharing_unit* s) {
        key k(s);
        tracker.insert({k, s});
    }
    void remove(sharing_unit* s) {
        key k(s);
        tracker.erase(k);
    }
};

void init_sharing_tracker_list();
sharing_tracker* get_sharing_tracker(int tracker_id);
#endif //STORAGE_SHARING_MODULE_SHARING_TRACKER_H
