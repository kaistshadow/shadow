//
// Created by csrc on 20. 12. 24..
//

#include "utils.h"

#include "../datatypes/datatype.h"
#include "../util/options.h"

#include <vector>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <sys/stat.h>


void _mkdir_storage_share() {
    if (mkdir("storage_share_datadir", 0776) == -1 && errno != EEXIST) {
        exit(-1);
    }
}
int init_fd_assignment_system() {
    _mkdir_storage_share();
    FILE* f = fopen("storage_share_datadir/storage_share_base_fd", "a");
    int basefd = dup(fileno(f));
    fclose(f);
	return basefd;
}
int try_assign_unused_fd() {
	static int basefd = init_fd_assignment_system();
    return dup(basefd);
}
int free_fd(int fd) {
    return close(fd);
}

// filemode parsing function
int parse_filemode(const char* mode) {
    int flag = 0;

    // parse fisrt letter
    switch(mode[0]) {
    case 'r':
        flag |= _FILEMODE_READ;
        break;
    case 'w':
        flag |= _FILEMODE_WRITE;
        flag |= _FILEMODE_CREATE;
        flag |= _FILEMODE_TRUNCATE;
        break;
    case 'a':
        flag |= _FILEMODE_READ;
        flag |= _FILEMODE_CREATE;
        flag |= _FILEMODE_APPEND;
        break;
    default:
        // error case
        return 0;
    }

    int i = 1;
    while(mode[i] != 0) {
        if (mode[i] == 'b') {
            flag |= _FILEMODE_BINARY;
        } else if (mode[i] == '+') {
            flag |= _FILEMODE_READ;
            flag |= _FILEMODE_WRITE;
        }
        i++;
    }

    return flag;
}

int random_name_base = 0;
size_t OFFSET_LIMIT = 10000;
struct dtype_filename {
    int fileno;
    size_t offset;
};
std::vector<struct dtype_filename> current_filenames;
// safe because it is locked on function 'sharing_unit* make_shared(memory_unit* m)'
std::string generate_filename(datatype* dtype, size_t* startoffset, size_t size) {
    if(get_storagemode() == STORAGE_SHARE_FIXED) {
        int tid = dtype->get_typeid();
        if (tid >= current_filenames.size()) {
            current_filenames.resize(tid + 1, {0, 0});
        } else if (current_filenames[tid].offset + size > OFFSET_LIMIT) {
            if (size > OFFSET_LIMIT) {
                printf("SHARING OFFSET TOO LOW\n");
                exit(-1);
            }
            current_filenames[tid].fileno++;
            current_filenames[tid].offset = 0;
        }
        char random_text[52];
        sprintf(random_text, "storage_share_datadir/%d-%d.txt", dtype->get_typeid(), current_filenames[tid].fileno);
        *startoffset = current_filenames[tid].offset;
        current_filenames[tid].offset += size;

        return std::string(random_text);
    } else if (get_storagemode() == STORAGE_SHARE_SEGMENT) {
        char random_text[52];
        sprintf(random_text, "storage_share_datadir/%d-%d.txt", dtype->get_typeid(), random_name_base++);
        *startoffset = 0;

        return std::string(random_text);
    } else {
        printf("storage mode is not properly initialized for sharing\n");
        exit(-1);
    }
}