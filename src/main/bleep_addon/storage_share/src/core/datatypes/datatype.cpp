//
// Created by csrc on 21. 1. 7..
//

#include "datatype.h"

#include "../data_unit/data_segment.h"

#include <regex>

/* Template for data splitting automata
 * start: receive Data (left) and its size (leftsize)
 * end: start point of the Data (right), which is splitting point dividing
 * */
int datatype::get_pivot(unsigned char* left, size_t* leftsize, unsigned char** right, size_t* rightsize) {
    return 0;
}
std::size_t datatype::hash_function(void* ptr, size_t size) {
    return 0;
}
// regex matched
simple_data* d0 = new simple_data();
bitcoin_block* d1 = new bitcoin_block();
bitcoin_rev* d2 = new bitcoin_rev();
datatype* get_matched_datatype(const char* filename) {
    std::regex r0(".*blk[0-9]{5}\.dat");
    std::regex r1(".*rev[0-9]{5}\.dat");
    if (std::regex_match(filename, r0)) {
        return d1;
    }
    if (std::regex_match(filename, r1)) {
        return d2;
    }
    return nullptr;
}

int simple_data::get_pivot(unsigned char* left, size_t* leftsize, unsigned char** right, size_t* rightsize) {
    if (*leftsize > 3) {
        *rightsize = *leftsize - 3;
        *right = &left[3];
        *leftsize = 3;
        return 1;
    }
    return 0;
}
std::size_t simple_data::hash_function(void* ptr, size_t size) {
    char buffer[4];
    if (size < 3) {
        memcpy(buffer, ptr, size);
    } else {
        memcpy(buffer, ptr, 3);
    }
    buffer[3] = 0;
    std::string s(buffer);
    return std::hash<std::string>()(s);
}

// https://learnmeabitcoin.com/technical/blkdat
int bitcoin_block::get_pivot(unsigned char* left, size_t* leftsize, unsigned char** right, size_t* rightsize) {
    if (*leftsize <= 8) {
        return 0;
    }
    size_t idx = 0;
    // magic byte (4 byte)
    unsigned char magic0[4] = {0xf9, 0xbe, 0xb4, 0xd9};
    unsigned char magic1[4] = {0x0b, 0x11, 0x09, 0x07};
    unsigned char magic2[4] = {0xfa, 0xbf, 0xb5, 0xda};
    if (!(memcmp(magic0, &left[idx], 4) || memcmp(magic1, &left[idx], 4) || memcmp(magic2, &left[idx], 4))) {
        // error case
        return 0;
    }
    idx += 4;
    // size (4 byte)
    size_t block_length = 0;
    for (int i = 3; i >= 0; i--) {
        block_length = block_length * 256 + (left[idx + i]);
    }
    if (block_length + 8 > *leftsize) {
        return 0;
    }
    *right = &left[block_length + 8];
    *rightsize = *leftsize - (block_length + 8);
    *leftsize = block_length + 8;
    return 1;
}
std::size_t bitcoin_block::hash_function(void* ptr, size_t size) {
    return 0;
}

// https://bitcoin.stackexchange.com/questions/57978/file-format-rev-dat
int bitcoin_rev::get_pivot(unsigned char* left, size_t* leftsize, unsigned char** right, size_t* rightsize) {
    if (*leftsize <= 8) {
        return 0;
    }
    size_t idx = 0;
    // magic byte (4 byte)
    unsigned char magic0[4] = {0xf9, 0xbe, 0xb4, 0xd9};
    unsigned char magic1[4] = {0x0b, 0x11, 0x09, 0x07};
    unsigned char magic2[4] = {0xfa, 0xbf, 0xb5, 0xda};
    if (!(memcmp(magic0, &left[idx], 4) || memcmp(magic1, &left[idx], 4) || memcmp(magic2, &left[idx], 4))) {
        // error case
        return 0;
    }
    idx += 4;
    // size (4 byte)
    size_t block_length = 0;
    for (int i = 3; i >= 0; i--) {
        block_length = block_length * 256 + (left[idx + i]);
    }
    if (block_length + 40> *leftsize) {
        return 0;
    }
    *right = &left[block_length + 40];
    *rightsize = *leftsize - (block_length + 40);
    *leftsize = block_length + 40;
    return 1;
}
std::size_t bitcoin_rev::hash_function(void* ptr, size_t size) {
    return 0;
}