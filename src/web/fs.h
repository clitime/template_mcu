#ifndef __FS_H__
#define __FS_H__

#ifdef HTTP_SERVER_DBG
#include <stdint.h>
typedef int8_t err_t;
#define ERR_ARG -16
#define ERR_OK 0
#define ERR_VAL -6
#else
#include "lwip/err.h"
#endif

#define FS_READ_EOF     -1
#define FS_READ_DELAYED -2

struct fs_file {
  const char *data;
  int len;
  int index;
  void *pextension;
};


err_t fs_open(struct fs_file *file, const char *name);
void fs_close(struct fs_file *file);
int fs_bytes_left(struct fs_file *file);

#endif /* __FS_H__ */
