#pragma once

#include <windows.h>

#include "types.h"

extern struct bios_pram_block bpb;  /* 全局的BPB单例 */

/*
 * FAT12/16引导扇区信息
 * 关闭内存对齐优化，方便一次性读取赋值
 */
struct fat_boot_sector
{
    u8 jump_ins[3];
    u8 OEM[8];
    /* BPB */
    u16 byte_per_sec;
    u8 sec_per_clus;
    u16 rsvd_sec_cnt;
    u8 num_fats;
    u16 root_ent_cnt;
    u16 tot_sec_16;
    u8 media;
    u16 sec_per_fat_16;
    u16 sec_per_track;
    u16 num_heads;
    u32 hidd_sec;
    u32 tot_sec_32;
    /* FAT12/16 EPBP */
    u8 drv_num;
    u8 reserved_1;
    u8 boot_sig;
    u32 vol_id;
    u8 vol_lab[11];
    u8 fs_type[8];
} __attribute__((packed));

/*
 * 简化过的BPB信息
 * 仅保留计算需要的内容
 */
struct bios_pram_block
{
    u16 byte_per_sec;   /* 每个扇区的字节数 */
    u8 sec_per_clus;    /* 每个簇的扇区数 */
    u16 rsvd_sec_cnt;   /* 保留扇区总数(包括引导扇区) */
    u8 num_fats;        /* FAT数量 */
    u16 root_ent_cnt;   /* 根目录条目数量上限 */
    u16 sec_per_fat;    /* 每个FAT占用扇区数 */
    u32 tot_sec;        /* 扇区总数 */
    /* FAT12/16 EPBP */
    u32 vol_id;
    u8 vol_lab[11];
    u8 fs_type[8];
};

void disk_open_vol(char vol_name);
DWORD disk_read(void *buffer, DWORD offset, DWORD size);
DWORD disk_write(void *buffer, DWORD offset, DWORD size);
void disk_read_bpb(struct bios_pram_block *bpb);
void disk_close();