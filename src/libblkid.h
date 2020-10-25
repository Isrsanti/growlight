// copyright 2012–2020 nick black
#ifndef GROWLIGHT_LIBBLKID
#define GROWLIGHT_LIBBLKID

#ifdef __cplusplus
extern "C" {
#endif

#include <blkid/blkid.h>

struct device;

int probe_blkid_superblock(const char *,blkid_probe *,struct device *);
int close_blkid(void);

#ifdef __cplusplus
}
#endif

#endif
