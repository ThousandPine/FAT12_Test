#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs.h"
#include "debug.h"

#define VOL_NAME 'h' /* 卷标，根据自己的分区进行调整 */

struct bios_pram_block bpb;

void parse_cmd(char *input, int *opt, char *cmd);

int main(int argc, char *argv[])
{
	disk_open_vol(VOL_NAME);
	disk_read_bpb(&bpb);

	const int SIZE = 1000;
	int opt;
	char arg[SIZE];
	char cmd[SIZE];

	while (1)
	{
		// gets_s(cmd, SIZE);
		gets(cmd);
		parse_cmd(cmd, &opt, arg);

		switch (opt)
		{
		case 1: /* ls */
			ls();
			break;

		case 2: /* cd  */
			cd(arg);
			break;

		case 3: /* mkdir */
			make_dir(arg);
			break;

		case 4: /* touch */
			touch(arg);
			break;

		case 5: /* rm */
			rm(arg);
			break;
		default:
			printf("Unknow command %s\n", cmd);
			break;
		}
	}

	disk_close();
	return 0;
}

void parse_cmd(char *cmd, int *opt, char *arg)
{
	int i = 0;
	int len = strlen(cmd);

	*opt = 0;
	while (i < len && isblank(cmd[i]))
		++i;
	while (i < len && isdigit(cmd[i]))
	{
		*opt = (*opt) * 10 + cmd[i++] - '0';
	}

	while (i < len && isblank(cmd[i]))
		++i;
	while (len > i && isblank(cmd[len - 1]))
		--len;
	if (i < len)
	{
		memcpy(arg, cmd + i, len - i);
		arg[len - i] = '\0';
	}
	else
		arg[0] = '\0';
	return;
}