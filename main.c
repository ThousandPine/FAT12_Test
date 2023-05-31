#include <stdio.h>
#include <string.h>

#include "disk.h"
#include "fs.h"
#include "debug.h"

#define VOL_NAME 'g' /* 卷标，根据自己的分区进行调整 */

struct bios_pram_block bpb; /* 定义全局PBP */

void parse_cmd(char *input, int *opt, char *cmd);
void print_bpb(struct bios_pram_block *bpb);

int main(int argc, char *argv[])
{
	disk_open_vol(VOL_NAME); /* 打开分区 */
	disk_read_bpb(&bpb);	 /* 读取BIOS参数块 */
	print_bpb(&bpb);

	const int SIZE = 1000;
	int opt;
	char arg[SIZE];
	char cmd[SIZE];

	while (1)
	{
		gets(cmd);
		parse_cmd(cmd, &opt, arg);

		switch (opt)
		{
		case 1: /* ls 显示文件列表 */
			ls();
			break;

		case 2: /* cd 切换目录 */
			cd(arg);
			break;

		case 3: /* mkdir 创建目录 */
			mkdir(arg);
			break;

		case 4: /* touch 创建文件 */
			touch(arg);
			break;

		case 5: /* rm 删除文件/目录 */
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

/* 解析命令内容 */
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

/* 输出bios_pram_block中的内容 */
void print_bpb(struct bios_pram_block *bpb)
{
	printf("文件系统类型: %s\n", bpb->fs_type);
	printf("卷标ID: %u\n", bpb->vol_id);
	printf("卷标名: %s\n\n", bpb->vol_lab);
	printf("每个扇区的字节数: %u\n", bpb->byte_per_sec);
	printf("每个簇的扇区数: %u\n", bpb->sec_per_clus);
	printf("保留扇区总数: %u\n", bpb->rsvd_sec_cnt);
	printf("FAT数量: %u\n", bpb->num_fats);
	printf("根目录条目数量上限: %u\n", bpb->root_ent_cnt);
	printf("每个FAT占用扇区数: %u\n", bpb->sec_per_fat);
	printf("扇区总数: %u\n", bpb->tot_sec);
}