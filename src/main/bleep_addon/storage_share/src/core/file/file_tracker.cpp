//
// Created by csrc on 20. 12. 24..
//

#include "file_tracker.h"

// try to replace/insert the entry for filename to f.
// if f is null, it just try to erase the entry for filename.
// return replaced old record.
file* file_tracker::replace(const char *filename, file *f) {
    std::string filenameStr(filename);

    file* peek = nullptr;
    auto it = tracker.find(filenameStr);
    if( it != tracker.end() ) {
        peek = it->second;
        if(f)
            it->second = f;
        else
            tracker.erase(it);
    } else {
        if(f)
            tracker.insert({filenameStr, f});
    }
    return peek;
}
file* file_tracker::lookup(const char *filename) {
    std::string filenameStr(filename);

    file* peek = nullptr;
    auto it = tracker.find(filenameStr);
    if( it != tracker.end() ) {
        peek = it->second;
    }
    return peek;
}
