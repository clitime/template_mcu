import os
import re
import gzip


def walksubdir(path):
    files = []
    for entry in os.scandir(path):
        if not entry.name.startswith('.') and entry.is_dir():
            files.extend(walksubdir(entry.path))
        if entry.is_file():
            files.append(entry.path)
    return files


def write_bytes(f, fsdata_c):
    for a in f:
        if write_bytes.i == 0:
            fsdata_c.write('\t')
        fsdata_c.write(str(hex(a)) + ', ')
        write_bytes.i += 1
        if write_bytes.i == 10:
            fsdata_c.write('\n')
            write_bytes.i = 0


write_bytes.i = 0


if __name__ == '__main__':
    fsdata_c = open('./fsdata.c', 'w')
    fsdata_c.write("#include \"fsdata.h\"\n\n")
    fsdata_c.write("#include <stddef.h>\n\n")

    fsdata_h = open('./fsdata.h', 'w')

    os.chdir("httpd")

    file_name = []

    for file in walksubdir("."):
        c_file = re.sub(r'\\', '/', file[1:])

        f_name = re.sub(r'/|\.', '_', c_file)
        file_name.append((f_name, len(c_file) + 1))

        fsdata_c.write('static const unsigned char data' + f_name + '[] = {\n')
        fsdata_c.write('\t/* ' + c_file + ' */\n\t')

        for a in list(map(hex, bytearray(c_file, 'utf8'))):
            fsdata_c.write(a + ', ')
        fsdata_c.write('0,\n')

        i = 0
        print('Add file: ', file)
        ext = re.search(r'.([a-z0-9]+)$', file, flags=re.ASCII)

        if ext[1] == 'js':
            ff = gzip.compress(open(file, 'rb').read())
            write_bytes(ff, fsdata_c)
        else:
            for f in open(file, 'rb'):
                write_bytes(f, fsdata_c)

        fsdata_c.write('0};\n\n')

    prev_file = ''
    prev_point = ''
    for name, length in file_name[:-1]:
        if prev_point == '':
            prev_point = 'NULL'
        else:
            prev_point = "&file" + prev_file

        prev_file = name

        fsdata_c.write('static const struct fsdata_file file' + name + ' = {')
        fsdata_c.write(prev_point + ', data' + name + ', ')
        fsdata_c.write('data' + name + ' + ' + str(length) + ', ')
        fsdata_c.write('sizeof(data' + name + ') - ' + str(length) + '};\n\n')
    else:
        prev_point = "&file" + prev_file
        (name, length) = file_name[-1]
        fsdata_c.write('const struct fsdata_file file' + name + ' = {')
        fsdata_c.write(prev_point + ', data' + name + ', ')
        fsdata_c.write('data' + name + ' + ' + str(length) + ', ')
        fsdata_c.write('sizeof(data' + name + ') - ' + str(length) + '};\n\n')

    fsdata_c.write('#define FS_NUMFILES ' + str(len(file_name)) + '\n')

    fsdata_c.close()

    fsdata_h.write('#ifndef __FSDATA_H__\n')
    fsdata_h.write('#define __FSDATA_H__\n')
    fsdata_h.write('\n')
    fsdata_h.write('#include "fs.h"\n')
    fsdata_h.write("\n")
    fsdata_h.write('extern const struct fsdata_file file' + name + ';\n')
    fsdata_h.write("\n")
    fsdata_h.write('#define FS_ROOT file' + name + '\n')
    fsdata_h.write("\n")
    fsdata_h.write("struct fsdata_file {\n")
    fsdata_h.write("    const struct fsdata_file *next;\n")
    fsdata_h.write("    const unsigned char *name;\n")
    fsdata_h.write("    const unsigned char *data;\n")
    fsdata_h.write("    const int len;\n")
    fsdata_h.write("};\n")
    fsdata_h.write("\n")
    fsdata_h.write("struct fsdata_file_noconst {\n")
    fsdata_h.write("    struct fsdata_file *next;\n")
    fsdata_h.write("    char *name;\n")
    fsdata_h.write("    char *data;\n")
    fsdata_h.write("    int len;\n")
    fsdata_h.write("};\n")
    fsdata_h.write("\n")
    fsdata_h.write("#endif /* __FSDATA_H__ */\n")
    fsdata_h.write("\n")

    fsdata_h.close()
