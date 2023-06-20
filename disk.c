#include <stdio.h>

#include "disk.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

static char _vol[7] = "\\\\.\\N:"; /* 卷标名 */
static HANDLE _handle = NULL;      /* 保存在内部的"文件指针" */

static const DWORD SEC_SIZE = 512; /* 预设定扇区大小 */

static void _lock_vol()
{
    DWORD dw;

    if (DeviceIoControl(_handle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dw, NULL) == 0)
    {
        printf("ERROR: disk::lock code[%lu]\n", GetLastError());
        exit(-1);
    }
}

static void _unlock_vol()
{
    DWORD dw;

    if (DeviceIoControl(_handle, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dw, NULL) == 0)
    {
        printf("ERROR: disk::lock code[%lu]\n", GetLastError());
        exit(-1);
    }
}

static DWORD _row_read(void *buffer, DWORD offset, DWORD size)
{
    DWORD readsize = 0;
    OVERLAPPED over = {0};
    over.Offset = offset;

    if (size != 0 && ReadFile(_handle, buffer, size, &readsize, &over) == 0)
    {
        printf("ERROR: disk::write code[%lu] buffer[%p] offset[%lu] size[%lu] \n", GetLastError(), buffer, offset, size);
        disk_close();
        exit(-1);
    }
    return readsize;
}

static DWORD _row_write(void *buffer, DWORD offset, DWORD size)
{
    DWORD writeensize = 0;
    OVERLAPPED over = {0};
    over.Offset = offset;

    _lock_vol();
    if (size != 0 && WriteFile(_handle, buffer, size, &writeensize, &over) == 0)
    {
        printf("ERROR: disk::write code[%lu] buffer[%p] offset[%lu] size[%lu] \n", GetLastError(), buffer, offset, size);
        _unlock_vol();
        disk_close();
        exit(-1);
    }
    _unlock_vol();

    return writeensize;
}

/* ============================================================== */

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
void disk_read(void *buffer, DWORD offset, DWORD size)
{
    u8 tmp_buff[SEC_SIZE]; /* 临时缓冲区 */
    DWORD buff_off = 0;    /* 当前读取的进度 */

    /*
     * 原版的API只能按照扇区读写
     * 也就是说offset和size都必须是512的整数倍，否则就会报错
     * 所以需要一些额外的操作来实现和Linux一样自由读写的效果
     * FK U Microsoft
     */

    /*
     * 起始地址未对齐扇区
     * 先读取属于该扇区内的数据
     */
    if (offset % SEC_SIZE != 0)
    {
        DWORD sec_off = offset % SEC_SIZE;               /* 扇区内偏移 */
        DWORD read_size = min(size, SEC_SIZE - sec_off); /* 该扇区内要读取的数据大小 */

        _row_read(tmp_buff, offset - sec_off, SEC_SIZE); /* 先读入临时缓冲区 */
        memcpy(buffer, tmp_buff + sec_off, read_size);

        buff_off += read_size;
    }
    /*
     * 读取中间包含的完整扇区
     */
    if (size - buff_off >= SEC_SIZE)
    {
        DWORD sec_cnt = (size - buff_off) / SEC_SIZE; /* 包含的扇区数 */
        DWORD read_size = SEC_SIZE * sec_cnt;

        _row_read((u8 *)buffer + buff_off, offset + buff_off, read_size);

        buff_off += read_size;
    }
    /*
     * 读取末尾地址未对其的扇区
     */
    if (buff_off < size)
    {
        DWORD read_size = size - buff_off;

        _row_read(tmp_buff, offset + buff_off, SEC_SIZE);
        memcpy((u8 *)buffer + buff_off, tmp_buff, read_size);
    }
}

/*
 * 数据写入分区
 */
void disk_write(void *buffer, DWORD offset, DWORD size)
{
    u8 tmp_buff[SEC_SIZE]; /* 临时缓冲区 */
    DWORD buff_off = 0;    /* 当前读取的进度 */

    /*
     * 进行非扇区对其的写入，原理与disk_read相似
     * 不过在写入非完整扇区数据时需要采取“读取、部分修改、写回”的策略
     * 使其不会破坏无关的数据
     */

    /*
     * 起始地址未对齐扇区
     * 进行部分写入
     */
    if (offset % SEC_SIZE != 0)
    {
        DWORD sec_off = offset % SEC_SIZE;               /* 扇区内偏移 */
        DWORD read_size = min(size, SEC_SIZE - sec_off); /* 该扇区内要读取的数据大小 */

        _row_read(tmp_buff, offset - sec_off, SEC_SIZE);  /* 先将原数据读入临时缓冲区 */
        memcpy(tmp_buff + sec_off, buffer, read_size);    /* 对缓冲区数据进行部分修改 */
        _row_write(tmp_buff, offset - sec_off, SEC_SIZE); /* 把被部分修改过的数据写回扇区 */

        buff_off += read_size;
    }
    /*
     * 写入中间包含的完整扇区
     */
    if (size - buff_off >= SEC_SIZE)
    {
        DWORD sec_cnt = (size - buff_off) / SEC_SIZE; /* 包含的扇区数 */
        DWORD read_size = SEC_SIZE * sec_cnt;

        _row_write((u8 *)buffer + buff_off, offset + buff_off, read_size);

        buff_off += read_size;
    }
    /*
     * 写入末尾地址未对其的扇区
     */
    if (buff_off < size)
    {
        DWORD read_size = size - buff_off;

        _row_read(tmp_buff, offset + buff_off, SEC_SIZE);
        memcpy(tmp_buff, (u8 *)buffer + buff_off, read_size);
        _row_write(tmp_buff, offset + buff_off, SEC_SIZE);
    }
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