//
// Created by csrc on 20. 12. 24..
//

#ifndef STORAGE_SHARING_MODULE_UTILS_H
#define STORAGE_SHARING_MODULE_UTILS_H

#include <string>

class datatype;

void init_fd_assignment_system(int bleepProcessId);
int try_assign_unused_fd();
int free_fd(int fd);

// file flags
#define _FILEMODE_READ      0x000001
#define _FILEMODE_WRITE     0x000010
#define _FILEMODE_CREATE    0x000100
#define _FILEMODE_TRUNCATE  0x001000
#define _FILEMODE_APPEND    0x010000
// binary flag is ignored on POSIX system, see https://man7.org/linux/man-pages/man3/fopen.3.html
#define _FILEMODE_BINARY    0x100000
int parse_filemode(const char* mode);

std::string generate_filename(datatype* dtype, size_t* startoffset);

#endif //STORAGE_SHARING_MODULE_UTILS_H
