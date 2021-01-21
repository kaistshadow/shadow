#include "bleep_addon.h"

// bleep_addon.h and cpp is for interfaces between Shadow and bleep-addon module.
// So, do not write functionality logic for each tasks in this code,
// just make submodule directories and separate them to other submodules.

/* bitcoin_mine_support */
#include "bitcoin_mine_support/coinflip_validation.h"
/* storage-sharing */
#include "storage_share/src/core/util/utils.h"
#include "storage_share/src/core/file_control.h"

#include <vector>

/* storage-sharing */
// one file control per node
std::vector<file_control*> global_file_controls;
int global_file_controls_size = 0;
file_control* get_file_control(unsigned int bleep_process_id) {
    if (bleep_process_id >= global_file_controls_size) {
        global_file_controls.resize(bleep_process_id + 1, nullptr);
        global_file_controls_size = bleep_process_id + 1;
    }
    file_control* fc = global_file_controls[bleep_process_id];
    if (fc == nullptr) {
        fc = new file_control();
        global_file_controls[bleep_process_id] = fc;
    }
    return fc;
}

extern "C"
{

/* bitcoin_mine_support */
void bleep_addon_bitcoin_register_hash(const char hash[]) {
    return bitcoin_mine_support().register_hash(hash);
}
int bleep_addon_bitcoin_check_hash(const char hash[]) {
    return bitcoin_mine_support().check_hash(hash);
}

/* storage_share */
FILE * bleep_addon_fopen (unsigned int bleepProcessID, const char * filename, const char * mode, char* f_dtype_mismatch) {
    file_control* fc = get_file_control(bleepProcessID);
    return fc->fopen(filename, mode, f_dtype_mismatch);
}
FILE * bleep_addon_fdopen(unsigned int bleepProcessID, int fd, const char *mode, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return nullptr;
}
int bleep_addon_fclose (unsigned int bleepProcessID, FILE * stream) {
    file_control* fc = get_file_control(bleepProcessID);
    return fc->fclose(stream);
}
int bleep_addon_feof (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_fflush (unsigned int bleepProcessID, FILE * stream) {
    file_control* fc = get_file_control(bleepProcessID);
    int res = fc->fflush(stream);
    return res;
}
int bleep_addon_remove (unsigned int bleepProcessID, const char * filename, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
FILE * bleep_addon_freopen (unsigned int bleepProcessID, const char * filename, const char * mode, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return nullptr;
}
int bleep_addon_rename (unsigned int bleepProcessID, const char * oldname, const char * newname, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_fseek (unsigned int bleepProcessID, FILE * stream, long int offset, int origin) {
    file_control* fc = get_file_control(bleepProcessID);
    return fc->fseek(stream, offset, origin);
}
int bleep_addon_fsetpos (unsigned int bleepProcessID, FILE * stream, const fpos_t * pos, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_fgetpos (unsigned int bleepProcessID, FILE * stream, fpos_t * pos, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
long int bleep_addon_ftell (unsigned int bleepProcessID, FILE * stream) {
    file_control* fc = get_file_control(bleepProcessID);
    return fc->ftell(stream);
}
void bleep_addon_rewind (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return;
}
int bleep_addon_fgetc (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
char * bleep_addon_fgets (unsigned int bleepProcessID, char * str, int num, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return nullptr;
}
int bleep_addon_fprintf (unsigned int bleepProcessID, FILE * stream, const char * format, char* f_dtype_mismatch, ... ) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_fputc (unsigned int bleepProcessID, int character, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_fputs (unsigned int bleepProcessID, const char * str, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
size_t bleep_addon_fread (unsigned int bleepProcessID, void * ptr, size_t size, size_t count, FILE * stream, char* f_dtype_mismatch) {
    file_control* fc = get_file_control(bleepProcessID);
    *f_dtype_mismatch = !(fc->has_descriptor(stream));
    if (*f_dtype_mismatch)
        return 0;
    size_t res = fc->fread(ptr, size, count, stream);
    return res;
}
int bleep_addon_fscanf (unsigned int bleepProcessID, FILE * stream, const char * format, char* f_dtype_mismatch, ... ) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
size_t bleep_addon_fwrite (unsigned int bleepProcessID, const void * ptr, size_t size, size_t count, FILE * stream, char* f_dtype_mismatch) {
    file_control* fc = get_file_control(bleepProcessID);
    *f_dtype_mismatch = !fc->has_descriptor(stream);
    if (*f_dtype_mismatch)
        return 0;
    size_t res = fc->fwrite(ptr, size, count, stream);
    return res;
}
int bleep_addon_getc (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_putc (unsigned int bleepProcessID, int character, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
void bleep_addon_setbuf (unsigned int bleepProcessID, FILE * stream, char * buffer, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
}
int bleep_addon_setvbuf (unsigned int bleepProcessID, FILE * stream, char * buffer, int mode, size_t size, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_ungetc (unsigned int bleepProcessID, int character, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_vfprintf (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch, const char * format, va_list arg ) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_vfscanf (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch, const char * format, va_list arg ) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
void bleep_addon_clearerr(unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
}
int bleep_addon_fileno(unsigned int bleepProcessID, FILE * stream) {
    file_control* fc = get_file_control(bleepProcessID);
    return fc->fileno(stream);
}
int bleep_addon_ferror(unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch) {
    // not implemented yet
    *f_dtype_mismatch = 1;
    return -1;
}
int bleep_addon_posix_fallocate(unsigned int bleepProcessID, int fd, off_t offset, off_t len) {
    file_control* fc = get_file_control(bleepProcessID);
    return fc->posix_fallocate(fd, offset, len);
}

}