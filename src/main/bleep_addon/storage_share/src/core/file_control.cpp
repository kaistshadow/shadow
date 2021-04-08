//
// Created by csrc on 20. 12. 24..
//

#include "file_control.h"

#include "file/file.h"
#include "util/utils.h"
#include "datatypes/datatype.h"

#include <cstdio>

// https://man7.org/linux/man-pages/man3/fopen.3.html
FILE * file_control::fopen(const char *filename, const char* mode, char* f_dtype_mismatch) {
    datatype* dtype = get_matched_datatype(filename);
    if (!dtype) {
        *f_dtype_mismatch = 1;
        return nullptr;
    }

    bool fCreate = false;
    file* f;

    // parse mode flag, check 'w' and 'a' mode
    int file_flag = parse_filemode(mode);
    if (!file_flag) {
        // error case
        return nullptr;
    }
    f = node_file_tracker.lookup(filename);
    if (!f) {
        // try to create file if it not exists
        if (file_flag & _FILEMODE_CREATE) {
            // if not, create new file with empty segments
            f = new file(filename, dtype);
            node_file_tracker.replace(filename, f);
            fCreate = true;
        } else {
            // error case: try to read non-existing file
            return nullptr;
        }
    }

    // request unused file descriptor for the opening, choose where to start with filemode flag
    file_descriptor* file_desc = node_file_descriptors.assign_file_descriptor(f, file_flag);
    if (!file_desc) {
        // error case
        if (fCreate) {
            node_file_tracker.replace(filename, nullptr);
            delete f;
        }
        return nullptr;
    }

    if (file_flag & _FILEMODE_TRUNCATE) {
        f->cleanse();
    }

    // return FILE ptr with filling proper fields which is used from this file control system
    return file_desc->get_file_info();
}
FILE * file_control::fdopen(int fd, const char *mode) {
    // not implemented yet
    return nullptr;
}
FILE * file_control::freopen ( const char * filename, const char * mode, FILE * stream ) {
    // close original stream, reopen
    // if filename is nullptr and freopen succeeds, do not close original stream and it returns original stream with changed modes.
    // not implemented yet
    return nullptr;
}

//https://man7.org/linux/man-pages/man3/fflush.3.html
int file_control::fflush(FILE *stream) {
    // update all data segment with fflush on the actual file
	// only do flush on writables.
    file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(stream->_fileno);
    if (!file_desc) {
        // error case
        return DTYPE_MISMATCH;
    }
    file* f = file_desc->get_file();
    if (!f) {
        // error case
        return -1;
    }
	// since every write is flushed directly by fclose for now, just return success.
    return 0;
}

// https://man7.org/linux/man-pages/man3/fclose.3.html
int file_control::fclose(FILE *stream) {
	// flushes stream, closes file descriptor
    int res = 0;

    // flush first before releasing the file descriptor
    res = fflush(stream);
    if (res == -1 || res == DTYPE_MISMATCH) {
        // error case
        return res;
    }

    // release file descriptor
    res = node_file_descriptors.release_file_descriptor(stream->_fileno);
    return res;
}

//https://man7.org/linux/man-pages/man3/feof.3.html
int file_control::feof(FILE *stream) {
    // if the virtual offset is the virtual size of the file, return true
    file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(stream->_fileno);
    if (!file_desc) {
        return DTYPE_MISMATCH;
    } else {
        return file_desc->check_eof();
    }
}
void file_control::clearerr(FILE * stream) {
    // not implemented yet
    return;
}
int file_control::fileno(FILE * stream) {
    file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(stream->_fileno);
    if (!file_desc) {
        return DTYPE_MISMATCH;
    } else {
        return stream->_fileno;
    }
}
int file_control::ferror(FILE * stream) {
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man3/remove.3p.html
int file_control::remove(const char *filename) {
    // if other descriptor uses the target file, do not remove and return -1
    // if successfully remove file, return 0
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man2/rename.2.html
int file_control::rename ( const char * oldname, const char * newname ) {
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man3/fseek.3.html
int file_control::fseek ( FILE * stream, long int offset, int origin ) {
	file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(stream->_fileno);
	if (!file_desc) {
        // error case
	    return DTYPE_MISMATCH;
	}
	size_t target_offset;
	switch(origin) {
	case SEEK_SET:
	    target_offset = offset;
	    break;
	case SEEK_CUR:
	    target_offset = stream->_offset + offset;
	    break;
	case SEEK_END:
	    target_offset = file_desc->get_file()->get_size() + offset;
	    break;
	default:
	    return -1;
	}
	file_desc->random_access(target_offset);
    return 0;
}
int file_control::fsetpos ( FILE * stream, const fpos_t * pos ) {
    // not implemented yet
    return -1;
}
int file_control::fgetpos ( FILE * stream, fpos_t * pos ) {
    // not implemented yet
    return -1;
}
long int file_control::ftell ( FILE * stream ) {
    file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(stream->_fileno);
    if (!file_desc) {
        // error case
        return DTYPE_MISMATCH;
    }
	return stream->_offset;
}
void file_control::rewind ( FILE * stream ) {
	fseek(stream, 0L, SEEK_SET);
    return;
}

//https://man7.org/linux/man-pages/man3/fgetc.3.html
int file_control::fgetc ( FILE * stream ) {
    // not implemented yet
    return 0;
}
char * file_control::fgets ( char * str, int num, FILE * stream ) {
    // not implemented yet
    return nullptr;
}
int file_control::getc ( FILE * stream ) {
    // not implemented yet
    return 0;
}
int file_control::ungetc ( int character, FILE * stream ) {
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man3/scanf.3.html
int file_control::fscanf ( FILE * stream, const char * format, ... ) {
    // not implemented yet
    return 0;
}
int file_control::vfscanf ( FILE * stream, const char * format, va_list arg ) {
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man3/fputc.3.html
int file_control::fputc ( int character, FILE * stream ) {
    // not implemented yet
    return 0;
}
int file_control::fputs ( const char * str, FILE * stream ) {
    // not implemented yet
    return 0;
}
int file_control::putc ( int character, FILE * stream ) {
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man3/fprintf.3p.html
int file_control::fprintf ( FILE * stream, const char * format, ... ) {
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man3/vfprintf.3p.html
int file_control::vfprintf ( FILE * stream, const char * format, va_list arg ) {
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man3/fread.3.html
size_t file_control::fread ( void * ptr, size_t size, size_t count, FILE * stream ) {
    // not implemented yet
	file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(stream->_fileno);
    return file_desc->read(ptr, size, count);
}
size_t file_control::fwrite ( const void * ptr, size_t size, size_t count, FILE * stream ) {
    // not implemented yet
    file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(stream->_fileno);
    return file_desc->write(ptr, size, count);
}

//https://man7.org/linux/man-pages/man3/setbuf.3.html
void file_control::setbuf ( FILE * stream, char * buffer ) {
    // not implemented yet
    return;
}
int file_control::setvbuf ( FILE * stream, char * buffer, int mode, size_t size ) {
    // not implemented yet
    return 0;
}

//https://man7.org/linux/man-pages/man3/posix_fallocate.3.html
int file_control::posix_fallocate(int fd, off_t offset, off_t len) {
    file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(fd);
    if (!file_desc) {
        return DTYPE_MISMATCH;
    }
    return file_desc->padding(offset, len);
}

char file_control::has_descriptor(FILE *stream) {
    file_descriptor* file_desc = node_file_descriptors.get_file_descriptor(stream->_fileno);
    if (!file_desc) {
        return 0;
    }
    return 1;
}

std::ostream& operator<<(std::ostream& os, const file_control& data_) {
    os << "debug";
    return os;
}
std::istream& operator>>(std::istream& is, const file_control& data_) {
    char buffer[6];
    buffer[5] = 0;
    is.read(buffer, 5);
    std::cout<<"istream test: "<<buffer;
    return is;
}