// copyright 2012–2020 nick black
#ifndef GROWLIGHT_GPT
#define GROWLIGHT_GPT

#ifdef __cplusplus
extern "C" {
#endif

#include <wchar.h>
#include <stdint.h>

#define GUIDSIZE 16	// 128 opaque bits

struct device;

// Pass the block device
int new_gpt(struct device *);
int zap_gpt(struct device *);
int add_gpt(struct device *,const wchar_t *,uintmax_t,uintmax_t,unsigned long long);

// Pass the partition
int del_gpt(const struct device *);
int name_gpt(struct device *,const wchar_t *);
int uuid_gpt(struct device *,const void *);
int flag_gpt(struct device *,uint64_t,unsigned);
int flags_gpt(struct device *,uint64_t);
int code_gpt(struct device *,unsigned long long);

uintmax_t first_gpt(const struct device *);
uintmax_t last_gpt(const struct device *);

#ifdef __cplusplus
}
#endif

#endif
