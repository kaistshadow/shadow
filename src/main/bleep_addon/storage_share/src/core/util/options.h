//
// Created by csrc on 21. 4. 7..
//

#ifndef STORAGE_SHARING_MODULE_OPTIONS_H
#define STORAGE_SHARING_MODULE_OPTIONS_H

// For shadow usage
#ifdef __cplusplus
extern "C"
{
#endif

enum _StorageMode {
    STORAGE_SHARE_DISABLE,
    STORAGE_SHARE_SEGMENT,
    STORAGE_SHARE_FIXED,
};
typedef enum _StorageMode StorageMode;
const char* storagemode_toStr(StorageMode mode);
StorageMode storagemode_fromStr(const char* storageStr);

#ifdef __cplusplus
}
#endif

#endif //BLEEP_OPTIONS_H
