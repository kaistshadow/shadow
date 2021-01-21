//
// Created by csrc on 20. 12. 24..
//

#ifndef STORAGE_SHARING_MODULE_FILE_TRACKER_H
#define STORAGE_SHARING_MODULE_FILE_TRACKER_H

#include "file.h"

#include <unordered_map>
#include <string>

class file_tracker {
    std::unordered_map<std::string, file*> tracker;
public:

    // try to replace/insert the entry for filename to f.
    // if f is null, it just try to erase the entry for filename.
    // return replaced old record.
    file* replace(const char* filename, file* f);
    file* lookup(const char* filename);
};

#endif //STORAGE_SHARING_MODULE_FILE_TRACKER_H
