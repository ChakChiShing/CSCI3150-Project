#include "call.h"
#include <stdio.h>
#include "superblock.h"
#include "superblock.c"
#include "inode.h"
#include "inode.c"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#ifndef DIR
#define DIR 1
#endif


//#define SB_OFFSET 4096 /* The offset of superblock region*/
//#define INODE_OFFSET 8192
//#define DATA_OFFSET 10485760
//#define BLOCK_SIZE 4096 /* The si􏰓e per block */
//
//typedef struct _inode_
//{
//    int inode_number;
//    time_t creation_time;
//    int file_type;
//    int file_size;
//    int blk_num;
//    int direct_blk[2];  // directblk [0]
//    int indirect_blk;
//    int file_num;
//}inode;
//
//typedef struct dir_mapping
//{
//    char dir[20];
//    int inode_number;
//}DIR_NODE;
//
//typedef struct _super_block_
//{
//        int inode_offset;
//        int data_offset;
//        int max_inode;
//        int max_data_blk;
//        int next_available_inode;
//        int next_available_blk;
//        int blk_size;
//}superblock;


int next_dir(int fd, int i_number, char *dir_name)
{
    int next_inode = -1;
    inode* ip;
    ip = read_inode(fd, i_number);
    if(ip->file_type != DIR)
    {
        printf("not the file type !!!!Wrong path!\n");
        return -1;
    }

    DIR_NODE* p_block = (DIR_NODE* )malloc(BLOCK_SIZE);

    int block_number = ip->direct_blk[0];
    int currpos = lseek(fd,DATA_OFFSET + block_number * BLOCK_SIZE, SEEK_SET);
    read(fd, p_block, BLOCK_SIZE);
    
    for(int i=0;i<ip->file_num;i++)
    {
        if(strcmp(p_block[i].dir,dir_name)==0)
        {
            next_inode = p_block[i].inode_number;
            break;
        }
    }
    free(p_block);
    free(ip);
    return next_inode;
}

//given a path name
//find its inode number
int open_t(char *pathname)
{
	int inode_number;
    // write your code here.
    int fd = open("./HD", O_RDONLY);
    inode *root = read_inode(fd,0); // for getting the inode number of root
    if(root == NULL){
        return -1;
    }
    
    inode_number =0;
    char *name = pathname;
    char *dir_name = strtok(name,"/");
    
    while(dir_name!=NULL){
        inode_number = next_dir(fd,inode_number,dir_name);
        dir_name = strtok(NULL,"/");
        if(inode_number<0){
            return -1;
        }
    }
    close(fd);
	return inode_number;
}


// do calculation on file size  (inode->file_size)
int read_t(int inode_number, int offest, void *buf, int count)
{
    int read_bytes;
    if(inode_number<0){
        return -1;
    }
    int fd = open("./HD",O_RDONLY);
    inode *some_inode = read_inode(fd, inode_number);
    if(some_inode == NULL){
        return -1;
    }
    if(some_inode->file_type != 0){
        printf("not a file\n\n\n");
        return -1;
    }
    
    int end = count +offest;
    if(end <= some_inode->file_size){
        read_bytes = count;
    }
    else{
        read_bytes = some_inode->file_size - offest;
    }
    
    if(read_bytes < 0){
        read_bytes = 0;
    }
    
    if(read_bytes>0)
        {
            int pt=0;
            int ed, st;
            st = offest/BLOCK_SIZE;
            ed = (offest+read_bytes)/BLOCK_SIZE;
            offest -= st*BLOCK_SIZE;
            
            if(st==0 && ed>=0)
            {
                //printf("going to read dir blk 0 pt: %d\n",pt);
                int sz = BLOCK_SIZE-offest;
                sz = (sz<read_bytes)?sz:read_bytes;
                lseek(fd, DATA_OFFSET+some_inode->direct_blk[0]*BLOCK_SIZE+offest,SEEK_SET);
                read(fd,(char*)buf+pt,sz);
                pt+=sz;
                offest = 0;
                //printf("read dir blk 0 cur pt: %d\n \n",pt);
            }
            if(st<=1 && ed >=1)
            {
                //printf("going to read dir blk 1 pt: %d\n",pt);
                int t = read_bytes-pt;
                int sz = BLOCK_SIZE-offest;
                sz = (sz<t)?sz:t;
                lseek(fd, DATA_OFFSET+some_inode->direct_blk[1]*BLOCK_SIZE+offest,SEEK_SET);
                read(fd,(char*)buf+pt,sz);
                pt+=sz;
                offest=0;
                //printf("read dir blk 1 cur pt: %d\n \n",pt);
            }
            int indr_ind;
            for(int rdblk=(st<2)?2:st;rdblk<=ed;rdblk++)
            {
                indr_ind = rdblk-2;
                //printf("going to read indir blk %d pt %d\n",indr_ind,pt);
                lseek(fd, DATA_OFFSET+some_inode->indirect_blk*BLOCK_SIZE+indr_ind*sizeof(int), SEEK_SET);
                read(fd,&indr_ind,sizeof(int));
                //printf("indir blk at %d\n",indr_ind);
                int t = read_bytes-pt;
                int sz = BLOCK_SIZE-offest;
                sz = (sz<t)?sz:t;
                lseek(fd, DATA_OFFSET+indr_ind*BLOCK_SIZE+offest,SEEK_SET);
                read(fd,(char*)buf+pt,sz);
                pt+=sz;
                offest=0;
                //printf("read indir blk %d cur pt %d\n \n", rdblk-2, pt);
            }
        }
        close(fd);
	return read_bytes; 
}


