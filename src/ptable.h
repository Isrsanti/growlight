// copyright 2012–2021 nick black
#ifndef GROWLIGHT_PTABLE
#define GROWLIGHT_PTABLE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct device;

#define MBR_OFFSET 440u

// Create the given type of partition table on this device
int make_partition_table(struct device *,const char *);

// Wipe the partition table (make it unrecognizable, preferably by overwriting
// it with zeroes). If a ptype is specified, it is assumed that this partition
// table type is being used, and we will zero out according to the specified
// type, even if it doesn't match the detected type (very dangerous!). If no
// type is specified, the detected type, if it exists, is used.
int wipe_ptable(struct device *,const char *);

int add_partition(struct device *,const wchar_t *,uintmax_t,uintmax_t,unsigned long long);
int wipe_partition(const struct device *);
int name_partition(struct device *,const wchar_t *);
int uuid_partition(struct device *,const void *);
int check_partition(struct device *);
int partition_set_flags(struct device *,uint64_t);
int partition_set_flag(struct device *,uint64_t,unsigned);
int partition_set_code(struct device *,unsigned long long);
int partitions_named_p(const struct device *);

uintmax_t lookup_first_usable_sector(const struct device *);
uintmax_t lookup_last_usable_sector(const struct device *);

// Interface to kernel's BLKPG ioctl
int blkpg_add_partition(int,long long,long long,int,const char *);
int blkpg_del_partition(int,long long,long long,int,const char *);

#ifdef __cplusplus
}
#endif

#endif
