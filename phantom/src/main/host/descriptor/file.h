/*
 * The Shadow Simulator
 * See LICENSE for licensing information
 */

#ifndef SRC_MAIN_HOST_DESCRIPTOR_FILE_H_
#define SRC_MAIN_HOST_DESCRIPTOR_FILE_H_

#include <poll.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "main/core/support/definitions.h"
#include "main/host/syscall/kernel_types.h"

/* Opaque type representing a file-backed file descriptor. */
typedef struct _File File;

/* In order to operate on a file, you must first create one with file_new()
 * and open it with either file_open() or file_openat(). Internally, we use
 * OS-backed files to support the Shadow file descriptor API.
 *
 * There are two main types of functions supported by this API:
 * - The first set operates on the given File*. The file must have been
 *   created and the File* must be non-null.
 * - The second set operates on paths, and optionally includes a file object
 *   that represents a directory from which a relative path is computed.
 *   These calls usually end with "at". The directory File* can be null, in
 *   which case the current working directory (AT_FDCWD) will be used instead.
 */

// ************************
// Initialization and setup
// ************************

File* file_new(); // Close the file with descriptor_close()
File* file_dup(File* file, int* dupError);
int file_open(File* file, const char* pathname, int flags, mode_t mode, const char* workingDir);
int file_openat(File* file, File* dir, const char* pathname, int flags, mode_t mode,
                const char* workingDir);

// ************************
// Accessors
// ************************

/* Returns the flags that were used to open the file. */
int file_getFlags(File* file);

/* Returns the mode that was used to open the file. */
mode_t file_getMode(File* file);

/* Returns the linux-backed fd that shadow uses to perform the file operations.  */
int file_getOSBackedFD(File* file);

// ****************************************
// Operations that require a non-null File*
// ****************************************

ssize_t file_read(File* file, Host* host, void* buf, size_t bufSize);
ssize_t file_pread(File* file, Host* host, void* buf, size_t bufSize, off_t offset);
ssize_t file_preadv(File* file, Host* host, const struct iovec* iov, int iovcnt, off_t offset);
#ifdef SYS_preadv2
ssize_t file_preadv2(File* file, Host* host, const struct iovec* iov, int iovcnt, off_t offset,
                     int flags);
#endif
ssize_t file_write(File* file, const void* buf, size_t bufSize);
ssize_t file_pwrite(File* file, const void* buf, size_t bufSize, off_t offset);
ssize_t file_pwritev(File* file, const struct iovec* iov, int iovcnt, off_t offset);
#ifdef SYS_pwritev2
ssize_t file_pwritev2(File* file, const struct iovec* iov, int iovcnt,
                      off_t offset, int flags);
#endif
int file_fstat(File* file, struct stat* statbuf);
int file_fstatfs(File* file, struct statfs* statbuf);
int file_fsync(File* file);
int file_fchown(File* file, uid_t owner, gid_t group);
int file_fchmod(File* file, mode_t mode);
int file_ftruncate(File* file, off_t length);
int file_fallocate(File* file, int mode, off_t offset, off_t length);
int file_fadvise(File* file, off_t offset, off_t len, int advice);
int file_flock(File* file, int operation);
int file_fsetxattr(File* file, const char* name, const void* value, size_t size,
                   int flags);
ssize_t file_fgetxattr(File* file, const char* name, void* value, size_t size);
ssize_t file_flistxattr(File* file, char* list, size_t size);
int file_fremovexattr(File* file, const char* name);
int file_sync_range(File* file, off64_t offset, off64_t nbytes,
                    unsigned int flags);
ssize_t file_readahead(File* file, off64_t offset, size_t count);
off_t file_lseek(File* file, off_t offset, int whence);
int file_getdents(File* file, struct linux_dirent* dirp, unsigned int count);
int file_getdents64(File* file, struct linux_dirent64* dirp,
                    unsigned int count);
int file_ioctl(File* file, unsigned long request, void* arg);
int file_fcntl(File* file, unsigned long command, void* arg);
int file_poll(File* file, struct pollfd* pfd);

// ******************************************
// Operations where the dir File* may be null
// ******************************************

int file_fstatat(File* dir, const char* pathname, struct stat* statbuf, int flags,
                 const char* workingDir);
int file_fchownat(File* dir, const char* pathname, uid_t owner, gid_t group, int flags,
                  const char* workingDir);
int file_fchmodat(File* dir, const char* pathname, mode_t mode, int flags, const char* workingDir);
int file_futimesat(File* dir, const char* pathname, const struct timeval times[2],
                   const char* workingDir);
int file_utimensat(File* dir, const char* pathname, const struct timespec times[2], int flags,
                   const char* workingDir);
int file_faccessat(File* dir, const char* pathname, int mode, int flags, const char* workingDir);
int file_mkdirat(File* dir, const char* pathname, mode_t mode, const char* workingDir);
int file_mknodat(File* dir, const char* pathname, mode_t mode, dev_t dev, const char* workingDir);
int file_linkat(File* olddir, const char* oldpath, File* newdir, const char* newpath, int flags,
                const char* workingDir);
int file_unlinkat(File* dir, const char* pathname, int flags, const char* workingDir);
int file_symlinkat(File* dir, const char* linkpath, const char* target, const char* workingDir);
ssize_t file_readlinkat(File* dir, const char* pathname, char* buf, size_t bufsize,
                        const char* workingDir);
int file_renameat2(File* olddir, const char* oldpath, File* newdir, const char* newpath,
                   unsigned int flags, const char* workingDir);
#ifdef SYS_statx
int file_statx(File* dir, const char* pathname, int flags, unsigned int mask,
               struct statx* statxbuf, const char* workingDir);
#endif

#endif /* SRC_MAIN_HOST_DESCRIPTOR_FILE_H_ */
