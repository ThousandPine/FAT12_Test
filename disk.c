#include <stdio.h>

#include "disk.h"

static char _vol[7] = "\\\\.\\N:";
static HANDLE _handle = NULL;

/*  
 * 打开分区
 * vol_name: 分区卷标
 */
void disk_open_vol(char vol_name)
{
    disk_close();

    _vol[4] = vol_name;
    _handle = CreateFile((LPCSTR)_vol, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (_handle == INVALID_HANDLE_VALUE)
    {
        printf("ERROR: disk::open_vol can not open %s", _vol);
        disk_close();
        exit(-1);
    }
}

/* 
 * 读取分区数据
 */
DWORD disk_read(void *buffer, DWORD offset, DWORD size)
{
    DWORD readsize;
    OVERLAPPED over = {0};
    over.Offset = offset;

    /*
     * 偏移非512整数倍
     * 
     */
    if(offset % 512 != 0)
    {
        
    }

    if (size % 512 == 0 && offset % 512 == 0)
    {
        if (ReadFile(_handle, buffer, size, &readsize, &over) == 0)
        {
            puts("ERROR: disk::read");
            disk_close();
            exit(-1);
        }
    }
    /*
     * 
     */
    else
    {
        u8 buff[512];

        if (ReadFile(_handle, buff, 512, &readsize, &over) == 0)
        {
            puts("ERROR: disk::read");
            disk_close();
            exit(-1);
        }

        memcpy(buffer, buff, size);
    }

    return readsize;
}

/*
 * 数据写入分区
 */
DWORD disk_write(void *buffer, DWORD offset, DWORD size)
{
    /*
     * 未测试能否写入小于512字节的数据 
     */
    DWORD writeensize;
    OVERLAPPED over = {0};
    over.Offset = offset;
    if (WriteFile(_handle, buffer, size, &writeensize, &over) == 0)
    {
        puts("ERROR: disk::write");
        disk_close();
        exit(-1);
    }

    return writeensize;
}

/*
 * 读取引导扇区中的BIOS参数块
 */
void disk_read_bpb(struct bios_pram_block *bpb)
{
    struct fat_boot_sector bs;
    disk_read(&bs, 0, sizeof(bs));

    bpb->byte_per_sec = bs.byte_per_sec;
    bpb->sec_per_clus = bs.sec_per_clus;
    bpb->rsvd_sec_cnt = bs.rsvd_sec_cnt;
    bpb->num_fats = bs.num_fats;
    bpb->root_ent_cnt = bs.root_ent_cnt;
    bpb->sec_per_fat = bs.sec_per_fat_16;
    bpb->tot_sec = bs.tot_sec_16 != 0 ? bs.tot_sec_16 : bs.tot_sec_32;
    bpb->vol_id = bs.vol_id;
    memcpy(bpb->vol_lab, bs.vol_lab, sizeof(bpb->vol_lab));
    memcpy(bpb->fs_type, bs.fs_type, sizeof(bpb->fs_type));

}

/*
 * 释放资源
 */
void disk_close()
{
    if (_handle != NULL)
        CloseHandle(_handle);
    _vol[4] = 'N';
    _handle = NULL;
}