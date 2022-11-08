#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MBR_SIGNATURE_OFS 0x1FE
#define MBR_PART_TAB_OFS 0x1BE

#define PART_TAB_SIZE 16
#define PART_TAB_ACTIVE_OFS 0x000
#define PART_TAB_TYPE_OFS 0x004

#define PART_ACTIVE_MASK 0x80

bool mbr_signature_valid(const uint8_t *mbr_sect);
int mbr_set_partition_type(int device, uint8_t type);
int mbr_set_partition_boot_flag(int device, bool bootable);
