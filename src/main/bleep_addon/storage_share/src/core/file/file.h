//
// Created by csrc on 20. 12. 24..
//

#ifndef STORAGE_SHARING_MODULE_FILE_H
#define STORAGE_SHARING_MODULE_FILE_H

#include "../datatypes/datatype.h"
#include "../data_unit/data_segment.h"
#include "../share/sharing_tracker.h"

#include <string>

// virtual view of file
class file{
    std::string filename;
    size_t size;
    int refcnt;

    std::map<long int, data_segment*> segments;

    datatype* dtype;

    void _debug_print();

public:
    file(const char* filename, datatype* dtype);
    long int get_size() const { return size; }

    // virtual file reference count is used for success check of file deletion
    int increase_refcnt() { return ++refcnt; }
    int decrease_refcnt() { return --refcnt; }

    std::map<long int, data_segment*>::iterator get_segment(long int offset);
    std::map<long int, data_segment*>::iterator get_end();

    // clear data which is local or shared with refCnt==1. shared and refCnt>1 data is not erased.
    void cleanse();

    std::map<long int, data_segment*>::iterator create_new_segment(size_t initial_size);
	void update_size();

    void fix_covered_segment(std::map<long int, data_segment*>::iterator it, size_t bytesErased);

    // CAUTION: remains safe when the file control system is accessed by only one thread at the same time.
    void divide_local_segment(std::map<long int, data_segment*>::iterator it);
};
#endif //STORAGE_SHARING_MODULE_FILE_H
