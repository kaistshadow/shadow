//
// Created by csrc on 21. 1. 7..
//

#ifndef STORAGE_SHARING_MODULE_DATATYPE_H
#define STORAGE_SHARING_MODULE_DATATYPE_H

#include <cstdio>

class data_segment;
class memory_unit;

#ifndef DTYPE_MISMATCH
#define DTYPE_MISMATCH (-43)
#endif
#define DATATYPE_NONE           0
class datatype {
public:
    virtual ~datatype() = default;
    virtual int get_typeid() const {return DATATYPE_NONE;}
    // get split point, return result number for subtype of the data
    virtual int get_pivot(unsigned char* left, size_t* leftsize, unsigned char** right, size_t* rightsize);
    virtual std::size_t hash_function(void* ptr, size_t size);
};
datatype* get_matched_datatype(const char* filename);

#define DATATYPE_SIMPLE_DATA    1
class simple_data :  public datatype {
public:
    ~simple_data() override = default;
    int get_typeid() const override {return DATATYPE_SIMPLE_DATA;}
    int get_pivot(unsigned char* left, size_t* leftsize, unsigned char** right, size_t* rightsize) override;
    std::size_t hash_function(void* ptr, size_t size) override;
};

#define DATATYPE_BITCOIN_BLOCK  2
class bitcoin_block : public datatype {
public:
    ~bitcoin_block() override = default;
    int get_typeid() const override {return DATATYPE_BITCOIN_BLOCK;}
    int get_pivot(unsigned char* left, size_t* leftsize, unsigned char** right, size_t* rightsize) override;
    std::size_t hash_function(void* ptr, size_t size) override;
};

#define DATATYPE_BITCOIN_REV  3
class bitcoin_rev : public datatype {
public:
    ~bitcoin_rev() override = default;
    int get_typeid() const override {return DATATYPE_BITCOIN_REV;}
    int get_pivot(unsigned char* left, size_t* leftsize, unsigned char** right, size_t* rightsize) override;
    std::size_t hash_function(void* ptr, size_t size) override;
};

#endif //STORAGE_SHARING_MODULE_DATATYPE_H
