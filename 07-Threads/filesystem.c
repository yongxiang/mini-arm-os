#include "filesystem.h"
#include <stdint.h>
#include <string.h>
#include "hash-djb2.h"
#include "malloc.h"


#define MAX_FS 16
extern void print_str(char *s);
struct fs_t {
    uint32_t hash;
    fs_open_dir_t dcb;
    void * opaque;
};

static struct fs_t fss[MAX_FS];

__attribute__((constructor)) void fs_init()
{
    memset(fss, 0, sizeof(fss));   // filesystem memory set
    print_str("fs_init\n");
}

int register_fs(const char * mountpoint,fs_open_dir_t dir_callback, void * opaque)
{

    int i;
    for (i = 0; i < MAX_FS; i++) {
        if (!fss[i].dcb) {
            fss[i].hash = hash_djb2((const uint8_t *) mountpoint, -1);

            fss[i].dcb = dir_callback;
            fss[i].opaque = opaque;
            return 0;
        }
    }

    return -1;
}

static int root_opendir()
{
    return OPENDIR_NOTFOUNDFS;
}

int fs_opendir(const char * path)
{
    const char * slash;
    uint32_t hash;
    int count = 0;
    int i;

    if ( path[0] == '\0' || (path[0] == '/' && path[1] == '\0') ) {
        return root_opendir();
    }

    while (path[0] == '/') {
        path++;
        count++;
    }

    slash = strchr(path, '/');

    if (!slash) {
        slash = path + strlen(path);

    }

    hash = hash_djb2((const uint8_t *) path, slash - path);

    if(*(slash) == '\0') {
        path = "";

    } else {
        path = slash + 1;
    }
    for (i = 0; i < MAX_FS; i++) {
        if (fss[i].hash == hash)
            return fss[i].dcb(fss[i].opaque, path);
    }

    return OPENDIR_NOTFOUNDFS;
}
