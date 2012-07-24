#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "gpt.h"
#include "growlight.h"

#define GUIDSIZE 16 // 128 bits

#define LBA_SIZE 512u
#define MBR_OFFSET 440u

static const unsigned char GPT_PROTECTIVE_MBR[LBA_SIZE - MBR_OFFSET] =
 "\x00\x00\x00\x00\x00\x00"	// 6 bytes of zeros
 "\x80"				// bootable (violation of GPT spec, but some
 				//  BIOS/MBR *and* UEFI won't boot otherwise)
 "\x00\x00\x00"			// CHS of first absolute sector
 "\xee"				// Protective partition type
 "\xff\xff\xff"			// CHS of last absolute sector
 "\x00\x00\x00\x00"		// LBA of first absolute sector
 "\xff\xff\xff\xff"		// Sectors in partition
 "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
 "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
 "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
 "\x55\xaa";			// MBR signature

// One LBA block, padded with zeroes at the end. 92 bytes.
typedef struct __attribute__ ((packed)) gpt_header {
	uint64_t signature;		// "EFI PART", 45 46 49 20 50 41 52 54
	uint32_t revision;		// Through UEFI 2.3.1: 00 00 01 00
	uint32_t headsize;		// Header size in little endian,
					// excludes padding: 5c 00 00 00 == 92
	uint32_t crc;			// crc32, 0 through headsize
	uint32_t reserved;		// must be 0
	uint64_t lba;			// location of this header
	uint64_t backuplba;		// location of backup header
	uint64_t first_usable;		// first usable lba
	uint64_t last_usable;		// last usable lba
	unsigned char disk_guid[GUIDSIZE];
	uint64_t partlba;		// partition entries lba for this copy
	uint32_t partcount;		// number of partition entries
	uint32_t partsize;		// size of partition entries
	uint32_t partcrc;		// crc32 of partition array
} gpt_header;

// 128-byte GUID partition entry. A GPT table must provide space for at least
// MINIMUM_GPT_ENTRIES (128) of these, equal to 16k (32 512-byte sectors, or
// 4 4096-byte sectors) in both copies of the GPT.
typedef struct __attribute__ ((packed)) gpt_entry {
	unsigned char type_guid[GUIDSIZE];
	unsigned char part_guid[GUIDSIZE];
	uint64_t first_lba;
	uint64_t last_lba;
	uint64_t flags;
	uint16_t name[36];	// 36 UTF-16LE code units
} gpt_entry;

#define MINIMUM_GPT_ENTRIES 128

int new_gpt(device *d){
	ssize_t r;
	int fd;

	if(d->layout != LAYOUT_NONE){
		diag("Won't create partition table on non-disk %s\n",d->name);
		return -1;
	}
	if(d->size % LBA_SIZE){
		diag("Won't create GPT on %juB disk %s\n",d->size,d->name);
		return -1;
	}
	if(d->size < LBA_SIZE + 2 * (LBA_SIZE + MINIMUM_GPT_ENTRIES * sizeof(gpt_entry))){
		diag("Won't create GPT on %juB disk %s\n",d->size,d->name);
		return -1;
	}
	if((fd = openat(devfd,d->name,O_RDWR|O_CLOEXEC|O_DIRECT)) < 0){
		diag("Couldn't open %s (%s?)\n",d->name,strerror(errno));
		return -1;
	}
	// protective MBR in first LBA
	if(lseek(fd,MBR_OFFSET,SEEK_SET) != MBR_OFFSET){
		diag("Couldn't seek to %u on %s (%s?)\n",MBR_OFFSET,d->name,strerror(errno));
		close(fd);
		return -1;
	}
	if((r = write(fd,GPT_PROTECTIVE_MBR,sizeof(GPT_PROTECTIVE_MBR))) < 0 ||
			r < (ssize_t)sizeof(GPT_PROTECTIVE_MBR)){
		diag("Couldn't write protective MBR on %s (%s?)\n",d->name,strerror(errno));
		close(fd);
		return -1;
	}
	// FIXME GPT header in second
	// 16k (32 512-byte sectors) minimum of GPT info (supports 128 partitions)
	if(close(fd)){
		diag("Error closing %d for %s (%s?)\n",fd,d->name,strerror(errno));
		return -1;
	}
	return 0;
}

int zap_gpt(device *d){
	if(d->layout != LAYOUT_NONE){
		diag("Won't zap partition table on non-disk %s\n",d->name);
		return -1;
	}
	if(d->blkdev.pttable == NULL || strcmp(d->blkdev.pttable,"gpt")){
		diag("No GPT on disk %s\n",d->name);
		return -1;
	}
	// FIXME
	return 0;
}
