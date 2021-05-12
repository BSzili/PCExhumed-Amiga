#ifndef VFS_H
#define VFS_H

// EDuke32 compatibility

#define buildvfs_fd int
#define buildvfs_fd_invalid (-1)
#define buildvfs_kfd int32_t
#define buildvfs_kfd_invalid (-1)
#define buildvfs_FILE FILE *

#define buildvfs_fwrite(p, s, n, fp) fwrite((p), (s), (n), (fp))
#define buildvfs_fclose(fp) fclose(fp)
#define buildvfs_exists(fn) (access((fn), F_OK) == 0)
#define buildvfs_fopen_write(fn) fopen((fn), "wb")
#define buildvfs_chdir chdir
#define buildvfs_getcwd getcwd
#define buildvfs_write(fd, p, s) write((fd), (p), (s))
#define buildvfs_close(fd) close(fd)
#define buildvfs_open_write(fn) open((fn), O_BINARY|O_TRUNC|O_CREAT|O_WRONLY, S_IREAD|S_IWRITE)
#define buildvfs_open_read(fn) open((fn), O_RDONLY|O_BINARY)
#define buildvfs_read(fd, p, s) read((fd), (p), (s))
#define buildvfs_mkdir(dir, x) Bmkdir(dir, x)

#define BUILDVFS_FIND_REC CACHE1D_FIND_REC
#define BUILDVFS_FIND_FILE CACHE1D_FIND_FILE
#define BUILDVFS_FIND_DIR CACHE1D_FIND_DIR

#endif
