//
// Created by csrc on 21. 1. 5..
//

#include "sharing_tracker.h"

#include <vector>
#include <cstring>
#include <cstdlib>

std::size_t hashfn::operator()(data_segment *obj) const {
    return obj->hash();
}
bool equalfn::operator()(data_segment* lhs, data_segment* rhs) const {
    size_t data_size = lhs->get_size();

    if (data_size != rhs->get_size())
        return false;

    unsigned char* lhs_data;
    unsigned char* rhs_data;
    if (lhs->get_flags() & _DATASEGMENT_SHARED) {
        lhs_data = (unsigned char*)malloc(sizeof(unsigned char) * data_size);
        lhs->read(lhs_data, data_size, 0);
    } else {
        lhs_data = ((memory_unit*)lhs)->get_data();
    }
    if (rhs->get_flags() & _DATASEGMENT_SHARED) {
        rhs_data = (unsigned char*)malloc(sizeof(unsigned char) * data_size);
        rhs->read(rhs_data, data_size, 0);
    } else {
        rhs_data = ((memory_unit*)rhs)->get_data();
    }
    int res = memcmp(lhs_data, rhs_data, data_size);
    if (lhs->get_flags() & _DATASEGMENT_SHARED) {
        free(lhs_data);
    }
    if (rhs->get_flags() & _DATASEGMENT_SHARED) {
        free(rhs_data);
    }
    return res == 0;
}

std::unordered_set<data_segment*, hashfn, equalfn>::iterator sharing_tracker::find(data_segment* s) {
    return tracker.find(s);
}
std::unordered_set<data_segment*, hashfn, equalfn>::iterator sharing_tracker::end() {
    return tracker.end();
}
void sharing_tracker::insert(sharing_unit* s) {
    tracker.insert(s);
}
void sharing_tracker::remove(sharing_unit* s) {
    tracker.erase(s);
}
void sharing_tracker::lock() {
    sharing_tracker_mutex.lock();
}
void sharing_tracker::unlock() {
    sharing_tracker_mutex.unlock();
}

sharing_tracker** tracker_list = nullptr;
void init_sharing_tracker_list() {
#define DATATYPE_MAX_CNT    10
    if (!tracker_list) {
        tracker_list = (sharing_tracker**)malloc(sizeof(void*) * DATATYPE_MAX_CNT);
        for (int i = 0; i < DATATYPE_MAX_CNT; i++) {
            tracker_list[i] = new sharing_tracker();
        }
    }
}
sharing_tracker* get_sharing_tracker(int tracker_id) {
    return tracker_list[tracker_id];
}