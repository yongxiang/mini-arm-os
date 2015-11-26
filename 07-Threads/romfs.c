#include <stdint.h>
#include "filesystem.h"
#include "romfs.h"
#include "hash-djb2.h"
#include "malloc.h"
extern void itoa(int i,char *s);
extern void print_str(char *s);
static uint32_t get_unaligned(const uint8_t * d)
{
    return ((uint32_t) d[0]) | ((uint32_t) (d[1] << 8)) | ((uint32_t) (d[2] << 16)) | ((uint32_t) (d[3] << 24));
}

static int romfs_open_dir(void * opaque, const char * path)
{
    uint32_t h = hash_djb2((const uint8_t *)path, -1);

    const uint8_t * romfs = (const uint8_t *) opaque;
    const uint8_t * meta;
    char * file_name;

    int result = 0;
    meta = romfs;
    char *meta_h_str = malloc(1*sizeof(char));
    for(meta=romfs; get_unaligned(meta) && get_unaligned(meta+4); meta += get_unaligned(meta+4)+12) {
        itoa(get_unaligned(meta+8),meta_h_str);
        if(get_unaligned(meta+8) == h) {

            file_name = (char*)(meta +12);
            print_str(file_name);
            print_str("    ");
        }
        result++;

    }
    return result;
}

void register_romfs(const char * mountpoint, const uint8_t * romfs)
{


    register_fs(mountpoint, romfs_open_dir, (void *) romfs);
}



