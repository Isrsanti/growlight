#include "main.h"
#include "gpt.h"
#include <cstring>

TEST_CASE("GPT") {

  // First eight bytes must be "EFI PART"
  SUBCASE("Signature") {
    gpt_header head;
    CHECK(0 == initialize_gpt(&head, 92, 0, 34));
    CHECK(0 == memcmp(&head.signature, "EFI PART", sizeof(head.signature)));
  }

  // Bytes 0x8--0xb must be 00 00 01 00 (1.00 by UEFI logic)
  SUBCASE("Revision") {
    gpt_header head;
    CHECK(0 == initialize_gpt(&head, 92, 0, 34));
    CHECK(0x00010000 == head.revision);
  }

  // Bytes 0xc--0xf must be >= 92, should be the logical block size
  SUBCASE("HeaderSize") {
    gpt_header head;
    CHECK(0 == initialize_gpt(&head, 92, 0, 34));
    CHECK(92 == head.headsize);
  }

  // Bytes 0x18--0x1f are the sector of the GPT primary, usually 1
  // Bytes 0x20--0x27 are the sector of the GPT alternate, provided as argument
  SUBCASE("GPTLBAs") {
    gpt_header head;
    CHECK(0 == initialize_gpt(&head, 92, 100000, 34));
    CHECK(1 == head.lba);
    CHECK(100000 == head.backuplba);
  }

  // Verify both CRCs, and the reserved area following HeaderCRC32
  SUBCASE("CRC32") {
    gpt_header head;
    CHECK(0 == initialize_gpt(&head, 92, 0, 34));
    // partition entry size must be a positive multiple of 128 (usually 128)
    CHECK(0 < head.partsize);
    CHECK(0 == (head.partsize % 128));
    // number of partition entries, usually 128 (MINIMUM_GPT_ENTRIES)
    CHECK(128 <= head.partcount);
    auto entries = new gpt_entry[head.partcount];
    update_crc(&head, entries);
    CHECK(0xc803405b == head.crc);
    CHECK(0 == head.reserved);
    CHECK(0x1ec50fb7 == head.partcrc);
    delete[] entries;
  }

  // Check that LBA of 512 sets header size and zeroes out remainder of sector.
  // The Unified Extensible Firmware Interface Specification, Version 2.3.1,
  // Errata C, June 27, 2012, states on page 104, in Table 16: "Size in bytes
  // of the GPT Header. The HeaderSize must be greater than 92 and must be less
  // than or equal to the logical block size." But everyone uses 92 :/.
  SUBCASE("FullLBA") {
    unsigned char sector[512];
    memset(sector, 0xff, sizeof(sector));
    gpt_header* head = reinterpret_cast<gpt_header*>(sector);
    CHECK(0 == initialize_gpt(head, sizeof(sector), 0, 34));
    CHECK(92 == head->headsize);
    for(size_t idx = 92 ; idx < sizeof(sector) ; ++idx){
      CHECK(0 == sector[idx]);
    }
  }

}