#ifndef BLEEP_ADDON_H
#define BLEEP_ADDON_H

// bleep_addon.h and cpp is for interfaces between Shadow and bleep-addon module.
// So, do not write functionality logic for each tasks in this code,
// just make submodule directories and separate them to other submodules.

// for StorageMode
#include "storage_share/src/core/util/options.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* for storage_share */
#ifndef DTYPE_MISMATCH
// temporary
#define DTYPE_MISMATCH (-43)
#endif

/* bitcoin_mine_support related interface */
void bleep_addon_bitcoin_register_hash(const char hash[]);
int bleep_addon_bitcoin_check_hash(const char hash[]);

/* storage_share */
void storage_share_dump_filestructure();
void storage_share_load_filestructure();
void init_storage_sharing(StorageMode mode);
FILE * bleep_addon_fopen (unsigned int bleepProcessID, const char * filename, const char * mode, char* f_dtype_mismatch);
FILE * bleep_addon_fdopen(unsigned int bleepProcessID, int fd, const char *mode, char* f_dtype_mismatch);
int bleep_addon_fclose (unsigned int bleepProcessID, FILE * stream);
int bleep_addon_feof (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_fflush (unsigned int bleepProcessID, FILE * stream);
int bleep_addon_remove (unsigned int bleepProcessID, const char * filename, char* f_dtype_mismatch);
FILE * bleep_addon_freopen (unsigned int bleepProcessID, const char * filename, const char * mode, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_rename (unsigned int bleepProcessID, const char * oldname, const char * newname, char* f_dtype_mismatch);
int bleep_addon_fseek (unsigned int bleepProcessID, FILE * stream, long int offset, int origin);
int bleep_addon_fsetpos (unsigned int bleepProcessID, FILE * stream, const fpos_t * pos, char* f_dtype_mismatch);
int bleep_addon_fgetpos (unsigned int bleepProcessID, FILE * stream, fpos_t * pos, char* f_dtype_mismatch);
long int bleep_addon_ftell (unsigned int bleepProcessID, FILE * stream);
void bleep_addon_rewind (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_fgetc (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch);
char * bleep_addon_fgets (unsigned int bleepProcessID, char * str, int num, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_fprintf (unsigned int bleepProcessID, FILE * stream, const char * format, char* f_dtype_mismatch, ... );
int bleep_addon_fputc (unsigned int bleepProcessID, int character, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_fputs (unsigned int bleepProcessID, const char * str, FILE * stream, char* f_dtype_mismatch);
size_t bleep_addon_fread (unsigned int bleepProcessID, void * ptr, size_t size, size_t count, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_fscanf (unsigned int bleepProcessID, FILE * stream, const char * format, char* f_dtype_mismatch, ... );
size_t bleep_addon_fwrite (unsigned int bleepProcessID, const void * ptr, size_t size, size_t count, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_getc (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_putc (unsigned int bleepProcessID, int character, FILE * stream, char* f_dtype_mismatch);
void bleep_addon_setbuf (unsigned int bleepProcessID, FILE * stream, char * buffer, char* f_dtype_mismatch);
int bleep_addon_setvbuf (unsigned int bleepProcessID, FILE * stream, char * buffer, int mode, size_t size, char* f_dtype_mismatch);
int bleep_addon_ungetc (unsigned int bleepProcessID, int character, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_vfprintf (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch, const char * format, va_list arg );
int bleep_addon_vfscanf (unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch, const char * format, va_list arg );
void bleep_addon_clearerr(unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_fileno(unsigned int bleepProcessID, FILE * stream);
int bleep_addon_ferror(unsigned int bleepProcessID, FILE * stream, char* f_dtype_mismatch);
int bleep_addon_posix_fallocate(unsigned int bleepProcessID, int fd, off_t offset, off_t len);

#ifdef __cplusplus
}
#endif

#endif
