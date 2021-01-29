//
// Created by csrc on 20. 12. 24..
//

#ifndef STORAGE_SHARING_MODULE_FILE_DESCRIPTOR_H
#define STORAGE_SHARING_MODULE_FILE_DESCRIPTOR_H

#include "file.h"

#include <vector>
#include <cstdio>

// file descriptor works as file cursor.
class file_descriptor {
    FILE file_info;
    file* f;
    int flags;

    // descriptor as cursor
    std::map<long int, data_segment*>::iterator cursor;
    size_t local_offset;
public:
    file_descriptor(file* f, int fd, int flags);
    file* get_file() {return f;}
    FILE* get_file_info() { return &file_info;}
	int check_eof();

	void sequential_access(long int offset);
	void random_access(size_t offset);

	size_t read(void * ptr, size_t size, size_t count);
    size_t write(const void * ptr, size_t size, size_t count);

    int padding(size_t offset, size_t size);
};
class file_descriptors {
    std::vector<file_descriptor*> fdlist;
public:
    file_descriptors() {
        fdlist.resize(10, nullptr);
    }
    file_descriptor* assign_file_descriptor(file* f, int flags);
    int release_file_descriptor(int fd);
    file_descriptor* get_file_descriptor(int fd);
};

#endif //STORAGE_SHARING_MODULE_FILE_DESCRIPTOR_H
