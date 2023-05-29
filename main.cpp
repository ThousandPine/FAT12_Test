#include <stdio.h>
#include <iostream>

#include "io.h"
#include "debug.h"

using namespace std;

#define VOL_NAME 'h' /* 分区卷标 */

int main(int argc, char *argv[])
{
	disk disk(VOL_NAME);
	bios_pram_block bpb;

	disk.read_bios_pram(bpb);

	bool ctn = true;
	while (ctn)
	{
		int opt;

		switch (opt)
		{
		case 1:	/* ls 列出文件 */

			break;

		case 2:	/* cd  */

			break;

		case 3:	/* mkdir */

			break;

		case 4:	/* touch */

			break;

		case 5:	/* rm */

			break;
		default:	
			break;
		}
	}

	return 0;
}
