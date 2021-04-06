//
// Created by csrc on 20. 12. 24..
//

#ifndef STORAGE_SHARING_MODULE_FILE_CONTROL_H
#define STORAGE_SHARING_MODULE_FILE_CONTROL_H

#include "datatypes/datatype.h"
#include "file/file_tracker.h"
#include "file/file_descriptor.h"
#include "share/sharing_tracker.h"

#include <cstdio>

// TEMPORARY: for now, it assumes that multiple threads does not access one file_control at the same time.
//            this means that only sharing_tracker should be synchronized, not other structures.
class file_control {
private:
    file_tracker node_file_tracker; // per node
    file_descriptors node_file_descriptors; // per node

public:
    file_control() {
        // TEMPORARY: fixed to bitcoin block logic
//        this->dtype = new bitcoin_block();
        init_sharing_tracker_list();
    }
    ~file_control() {
    }
//    file_control(sharing_tracker_base* sharing_tracker) : global_sharing_tracker(sharing_tracker) {}
    FILE * fopen ( const char * filename, const char * mode, char* f_dtype_match);
    FILE * fdopen(int fd, const char *mode);
    int fclose ( FILE * stream );
    int feof ( FILE * stream );
    int fflush ( FILE * stream );
    int remove ( const char * filename );
    FILE * freopen ( const char * filename, const char * mode, FILE * stream );
    int rename ( const char * oldname, const char * newname );

    int fseek ( FILE * stream, long int offset, int origin );
    int fsetpos ( FILE * stream, const fpos_t * pos );
    int fgetpos ( FILE * stream, fpos_t * pos );
    long int ftell ( FILE * stream );
    void rewind ( FILE * stream );

    int fgetc ( FILE * stream );
    char * fgets ( char * str, int num, FILE * stream );
    int fprintf ( FILE * stream, const char * format, ... );
    int fputc ( int character, FILE * stream );
    int fputs ( const char * str, FILE * stream );
    size_t fread ( void * ptr, size_t size, size_t count, FILE * stream );
    int fscanf ( FILE * stream, const char * format, ... );
    size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
    int getc ( FILE * stream );
    int putc ( int character, FILE * stream );
    void setbuf ( FILE * stream, char * buffer );
    int setvbuf ( FILE * stream, char * buffer, int mode, size_t size );
    int ungetc ( int character, FILE * stream );
    int vfprintf ( FILE * stream, const char * format, va_list arg );
    int vfscanf ( FILE * stream, const char * format, va_list arg );

    void clearerr(FILE * stream);
    int fileno(FILE * stream);
    int ferror(FILE * stream);

    int posix_fallocate(int fd, off_t offset, off_t len);

    char has_descriptor(FILE * stream);
};
typedef enum _StorageMode StorageMode;
enum _StorageMode {
    STORAGE_SHARE_DISABLE,
    STORAGE_SHARE_SEGMENT,
    STORAGE_SHARE_FIXED,
};
const char* storagemode_toStr(StorageMode mode);
StorageMode storagemode_fromStr(const char* storageStr);


#endif //STORAGE_SHARING_MODULE_FILE_CONTROL_H
