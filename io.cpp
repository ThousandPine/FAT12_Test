#include "io.h"
#include "debug.h"

disk::disk(char vol_name)
{
    this->open_vol(vol_name);
}

disk::~disk()
{
    this->close();
}

char disk::vol() { return _vol[4]; }

void disk::open_vol(char vol_name)
{
    this->close();

    _vol[4] = vol_name;
    _handle = CreateFile((LPCSTR)_vol, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (_handle == INVALID_HANDLE_VALUE)
    {
        printf("ERROR: disk::open_vol can not open %s", _vol);
        this->close();
        exit(-1);
    }
}
DWORD disk::read(void *buffer, DWORD offset, DWORD size)
{
    DWORD readsize;
    OVERLAPPED over = {0};
    over.Offset = offset;

    if (size >= 512)
    {
        if (ReadFile(_handle, buffer, size, &readsize, &over) == 0)
        {
            puts("ERROR: disk::read");
            this->close();
            exit(-1);
        }
    }
    /*
     * 直接读小于512字节的内容会报错
     * 所以需要先读取512字节然后再把需要的部分拷贝回去 
     */
    else
    {
        u8 buff[512];

        if (ReadFile(_handle, buff, 512, &readsize, &over) == 0)
        {
            puts("ERROR: disk::read");
            this->close();
            exit(-1);
        }

        memcpy(buffer, buff, size);
    }

    return readsize;
}
DWORD disk::write(void *buffer, DWORD offset, DWORD size)
{
    DWORD writeensize;
    OVERLAPPED over = {0};
    over.Offset = offset;
    if (WriteFile(_handle, buffer, size, &writeensize, &over) == 0)
    {
        puts("ERROR: disk::write");
        this->close();
        exit(-1);
    }

    return writeensize;
}
void disk::close()
{
    if (_handle != nullptr)
        CloseHandle(_handle);
    _vol[4] = 'N';
    _handle = nullptr;
}

void disk::read_bios_pram(bios_pram_block &bpb)
{
    fat_boot_sector bs;
    this->read(&bs, 0, sizeof(bs));

    bpb.byte_per_sec = bs.byte_per_sec;
    bpb.sec_per_clus = bs.sec_per_clus;
    bpb.rsvd_sec_cnt = bs.rsvd_sec_cnt;
    bpb.num_fats = bs.num_fats;
    bpb.root_ent_cnt = bs.root_ent_cnt;
    bpb.tot_sec = bs.tot_sec_16 != 0 ? bs.tot_sec_16 : bs.tot_sec_32;
    bpb.vol_id = bs.vol_id;
    memcpy(bpb.vol_lab, bs.vol_lab, sizeof(bpb.vol_lab));
    memcpy(bpb.fs_type, bs.fs_type, sizeof(bpb.fs_type));
}