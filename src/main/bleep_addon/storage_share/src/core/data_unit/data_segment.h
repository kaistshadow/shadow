//
// Created by csrc on 20. 12. 24..
//

#ifndef STORAGE_SHARING_MODULE_DATA_SEGMENT_H
#define STORAGE_SHARING_MODULE_DATA_SEGMENT_H

#include "../datatypes/datatype.h"

#include <map>
#include <utility>
#include <cstring>

class sharing_tracker;
class sharing_unit;

class data_segment {
protected:
    size_t size;
    // sharing related
#define _DATASEGMENT_SHARED	0x01    // = shared
#define _DATASEGMENT_SPLITED	0x10    // = well formated
    int flags;

    datatype* dtype;
    std::size_t hash_value = 0;
public:
    data_segment(datatype* dtype);
    data_segment(size_t initial_size, datatype* dtype);
    virtual ~data_segment() = default;

    size_t get_size() const {return size;}

    virtual size_t read(void* dest, size_t count, size_t local_offset) {return 0;}
    virtual size_t write(const void* ptr, size_t count, size_t local_offset) {return 0;}

    int get_flags() const { return flags; }
    datatype* get_typeptr() { return dtype; }
    virtual std::size_t hash();
};

class memory_unit;
class sharing_unit;

class memory_unit : public data_segment {
    unsigned char* data;
public:
    memory_unit(datatype* dtype) : data_segment(dtype) { data = nullptr; }
    memory_unit(size_t initial_size, datatype* dtype) : data_segment(initial_size, dtype) {
        data = (unsigned char*)malloc(sizeof(unsigned char) * initial_size);
        memset(data, 0, sizeof(char) * initial_size);
    }
    memory_unit(unsigned char* copy_target, size_t initial_size, datatype* dtype) : data_segment(initial_size, dtype) {
        data = (unsigned char*)malloc(sizeof(unsigned char) * initial_size);
        mempcpy(data, copy_target, initial_size);
    }
    // copy content from sharing unit u, delete u
    memory_unit(sharing_unit* u);
    ~memory_unit() override {
        free(data);
    }
    size_t read(void* dest, size_t count, size_t local_offset) override;
    size_t write(const void* ptr, size_t count, size_t local_offset) override;

    unsigned char* get_data() { return data; }
    std::size_t hash() override;
};
sharing_unit* make_shared(memory_unit* m);

class sharing_unit : public data_segment {
    std::string filename;
    size_t fileoffset;

    int reference_count;
public:
    // sharing unit only made from memory unit
    // find corresponding fileane and fileoffset. if not exists, register such offset in file with m->get_size()
    // delete m
    sharing_unit(memory_unit* m);
    ~sharing_unit() override {};
    size_t read(void* dest, size_t count, size_t local_offset) override;
    // cannot write shared file
    size_t write(const void* ptr, size_t count, size_t local_offset) override {return 0;}

    int reference() { return ++reference_count; }
    int unreference() { return --reference_count; }
    std::size_t hash() override;
};
memory_unit* make_local(sharing_unit* s);
#endif //STORAGE_SHARING_MODULE_DATA_SEGMENT_H