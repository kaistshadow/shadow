//
// Created by csrc on 21. 4. 7..
//

#include "options.h"

#include <string.h>

const char* storagemode_toStr(StorageMode mode) {
    switch (mode) {
        case STORAGE_SHARE_SEGMENT:
            return "segment";
        case STORAGE_SHARE_FIXED:
            return "fixed";
        case STORAGE_SHARE_DISABLE:
        default:
            return "disable";
    }
}
StorageMode storagemode_fromStr(const char* storageStr) {
    if(storageStr == nullptr) {
        return STORAGE_SHARE_DISABLE;
    } else if (strcmp(storageStr, "segment") == 0) {
        return STORAGE_SHARE_SEGMENT;
    } else if (strcmp(storageStr, "fixed") == 0) {
        return STORAGE_SHARE_FIXED;
    } else {
        return STORAGE_SHARE_DISABLE;
    }
}
StorageMode g_storagemode;
StorageMode get_storagemode() {
    return g_storagemode;
}
StorageMode set_storagemode(StorageMode mode) {
    g_storagemode = mode;
}