#ifndef __FSDATA_H__
#define __FSDATA_H__

#include "fs.h"

extern const struct fsdata_file file_style_css;

#define FS_ROOT file_style_css

struct fsdata_file {
    const struct fsdata_file *next;
    const unsigned char *name;
    const unsigned char *data;
    const int len;
};

struct fsdata_file_noconst {
    struct fsdata_file *next;
    char *name;
    char *data;
    int len;
};

#endif /* __FSDATA_H__ */

