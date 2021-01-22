//
// Created by csrc on 21. 1. 5..
//

#include "sharing_tracker.h"

#include <vector>
#include <cstring>
#include <cstdlib>

bool key::operator==(const key &other) const {
    size_t our_size = segment->get_size();
    size_t other_size = other.segment->get_size();
    if (our_size != other_size)
        return false;
    unsigned char* our_data;
    unsigned char* other_data;
    if (segment->get_flags() & _DATASEGMENT_SHARED) {
        our_data = (unsigned char*)malloc(sizeof(unsigned char) * our_size);
        segment->read(our_data, our_size, 0);
    } else {
        our_data = ((memory_unit*)segment)->get_data();
    }
    if (other.segment->get_flags() & _DATASEGMENT_SHARED) {
        other_data = (unsigned char*)malloc(sizeof(unsigned char) * other_size);
        other.segment->read(other_data, other_size, 0);
    } else {
        other_data = ((memory_unit*)other.segment)->get_data();
    }
    int res = memcmp(our_data, other_data, our_size);
    if (segment->get_flags() & _DATASEGMENT_SHARED) {
        free(our_data);
    }
    if (other.segment->get_flags() & _DATASEGMENT_SHARED) {
        free(other_data);
    }
    return res == 0;
}

std::unordered_map<key, sharing_unit*, hashfn>::iterator sharing_tracker::find(data_segment* s) {
    key k(s);
    return tracker.find(k);
}
std::unordered_map<key, sharing_unit*, hashfn>::iterator sharing_tracker::end() {
    return tracker.end();
}
void sharing_tracker::insert(sharing_unit* s) {
    key k(s);
    tracker.insert({k, s});
}
void sharing_tracker::remove(sharing_unit* s) {
    key k(s);
    tracker.erase(k);
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