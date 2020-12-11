#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <time.h>

#include "inode.h"
#include "superblock.h"

//########
inode* read_inode(int fd, int inode_number){
	inode* ip = malloc(sizeof(inode));
	int currpos=lseek(fd, INODE_OFFSET + inode_number * sizeof(inode), SEEK_SET);
	if(currpos<0){
		printf("Error: lseek()\n");
		return NULL;
	}
	
	//read inode from disk
	int ret = read(fd, ip, sizeof(inode));
	if(ret != sizeof (inode) ){
		printf("Error: read()\n");
		return NULL;
	}
	return ip;
}

void print_inode_info(inode* ip){
	printf("the inode information: \n");
	printf("inode_number:	%d\n", ip->inode_number);
	printf("creation_time:	%s", ctime(& ip->creation_time));
	printf("file_type:		%d\n", ip->file_type);
	printf("file_size:		%d\n", ip->file_size);
	printf("blk_num:	%d\n", ip->blk_num);
	printf("direct_blk[0]:	%d\n", ip->direct_blk[0]);
	printf("direct_blk[1]:	%d\n", ip->direct_blk[1]);
	printf("indirect_blk:	%d\n", ip->indirect_blk);
	printf("file_num:	%d\n", ip->file_num);
}

void print_inode_region(int fd, int inode_number){
	printf("the inode region on disk:\n");
	unsigned int buf[sizeof(inode) / 4];
	int currpos = lseek(fd, INODE_OFFSET + inode_number * sizeof(inode), SEEK_SET);
	read(fd, buf, sizeof(inode));
	int i;
	for (i = 0; i < sizeof(inode) / 4; i++){
		printf("%04x\n", buf[i]);
	}
}

//#############
void print_dir_mappings(int fd, int inode_number)
{
	inode* ip;
	ip = read_inode(fd, inode_number);
	if(ip->file_type != DIR)
	{
		printf("Wrong path!\n");
		return;
	}

	DIR_NODE* p_block = (DIR_NODE* )malloc(BLOCK_SIZE);
	// Consider that SFS only supports at most 100 inodes so that only direct_blk[0] will be used,
	// the implementation is much easier
	int block_number = ip->direct_blk[0];
	int currpos=lseek(fd, DATA_OFFSET + block_number * BLOCK_SIZE, SEEK_SET);
	read(fd, p_block, BLOCK_SIZE);

	int file_idx = 0;
	printf("dir \t inode_number\n");
	for(file_idx = 0; file_idx < ip->file_num; file_idx++)
	{
		printf("%s \t %d\n", p_block[file_idx].dir, p_block[file_idx].inode_number);
	}
	free(p_block);
}
