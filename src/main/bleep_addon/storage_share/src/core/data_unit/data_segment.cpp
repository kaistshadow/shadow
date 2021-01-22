//
// Created by csrc on 20. 12. 24..
//

#include "data_segment.h"

#include "../share/sharing_tracker.h"
#include "../util/utils.h"

#include <cassert>
#include <cstdio>

data_segment::data_segment(datatype* dtype) {
    size = 0;
    flags = 0;
    this->dtype =dtype;
}
data_segment::data_segment(size_t initial_size, datatype* dtype) {
    size = initial_size;
    flags = 0;
    this->dtype = dtype;
}
std::size_t data_segment::hash() {
    printf("This should not be called.\n");
    return 0;
}

memory_unit::memory_unit(sharing_unit* u) : data_segment(u->get_size(), u->get_typeptr()) {
    flags = u->get_flags() & ~_DATASEGMENT_SHARED;
    hash_value = u->hash();
    // copy
    data = (unsigned char*)malloc(sizeof(unsigned char) * u->get_size());
    u->read(data, u->get_size(), 0);
    // delete u
    sharing_tracker* tracker = get_sharing_tracker(u->get_typeptr()->get_typeid());
    // lock tracker
    tracker->lock();
    if (u->unreference() == 0) {
        tracker->remove(u);
        // unlock tracker
        tracker->unlock();
        delete u;
    } else {
        // unlock tracker
        tracker->unlock();
    }
}
size_t memory_unit::read(void* dest, size_t count, size_t local_offset) {
    if (local_offset >= size) {
        return 0;
    }
    size_t readcnt = count;
    if (size - local_offset < count) {
        readcnt = size - local_offset;
    }
    memcpy(dest, &data[local_offset], readcnt);
    return readcnt;
}
size_t memory_unit::write(const void* ptr, size_t count, size_t local_offset) {
    if (local_offset+count > size) {
        unsigned char* newdata = (unsigned char*)realloc(data, (local_offset+count));
        if (!newdata) {
            printf("fatal error occured on memory_unit:write\n");
            return 0;
        }
        data = newdata;
        if (local_offset > size) {
            memset(&data[size], 0, sizeof(unsigned char) * (local_offset - size));
        }
        size = local_offset+count;
    }
    memcpy(&data[local_offset], ptr, count);
    hash_value = 0;
//    flags &= ~_DATASEGMENT_SPLITED;
    return count;
}
std::size_t memory_unit::hash() {
    if (hash_value != 0)
        return hash_value;
    unsigned char* buffer = nullptr;
    buffer = (unsigned char*)realloc(buffer, sizeof(unsigned char) * (size + 1));
    buffer[size] = 0;
    read(buffer, size, 0);
    std::size_t res = dtype->hash_function(buffer, size);
    free(buffer);
    hash_value = res;
    return res;
}
sharing_unit* make_shared(memory_unit* m) {
    sharing_tracker* tracker = get_sharing_tracker(m->get_typeptr()->get_typeid());
    sharing_unit* res;
    sharing_unit* res0 = new sharing_unit(m);
    // lock tracker
    tracker->lock();
    auto it = tracker->find(m);
    if (it == tracker->end()) {
        tracker->insert(res0);
        res = res0;
    } else {
        res = it->second;
    }
    res->reference();
    // unlock tracker
    tracker->unlock();
    if (res != res0) {
        delete res0;
    }
    delete m;
    return res;
}

sharing_unit::sharing_unit(memory_unit* m) : data_segment(m->get_size(), m->get_typeptr()) {
    // parent field
    flags = m->get_flags() | _DATASEGMENT_SHARED;
    this->hash_value = m->hash();
    // shared_unit specific field
    filename = generate_filename(dtype, &fileoffset); // decide filename and fileoffset
    reference_count = 0;

    FILE* f = fopen(filename.c_str(), "r+b");
    if (f == nullptr) {
        f = fopen(filename.c_str(), "w+b");
    }
    fseek(f, fileoffset, SEEK_SET);
    fwrite(m->get_data(), sizeof(unsigned char), m->get_size(), f);
    fclose(f);
}
size_t sharing_unit::read(void *dest, size_t count, size_t local_offset) {
    FILE* f = fopen(filename.c_str(), "rb");
    fseek(f, fileoffset + local_offset, SEEK_SET);

    size_t readable = count;
    if (this->size - local_offset <= readable) {
        readable = this->size - local_offset;
    }
    int res = fread(dest, sizeof(unsigned char), readable, f);

    fclose(f);
    return res;
}
std::size_t sharing_unit::hash() {
    return hash_value;
}