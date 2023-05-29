#pragma once

#include <stdio.h>
#include <Windows.h>

#include "fat.h"

class disk
{
public:
    disk(char vol_name);
    ~disk();

    char vol();
    void open_vol(char vol_name);
    DWORD read(void *buffer, DWORD offset, DWORD size);
    DWORD write(void *buffer, DWORD offset, DWORD size);
    void close();
    void read_bios_pram(bios_pram_block &bpb);

private:
    char _vol[7] = "\\\\.\\N:";
    HANDLE _handle = nullptr;
};