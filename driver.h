#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<errno.h>
#include<string.h>

#define estream stderr
#define ostream stdout
#define istream stdin
long blocksize;
int filehinoneblock,diskhead;
struct diskInfo
{
        char diskname[56];
        int numfiles;
        int numfreeblocks;
};
struct fileInfo
{
	char filename[52];
	int startno;  //starting block number of file.
	int sizeinblocks;     //file size in blocks
	int endbytes; //available bytes in the end block of file
	//int fileno; //for how many files in disk
};
struct freetrack
{
  	char isfree;   //'0' for free and '1' for full
};
struct filepointers
{
  	int next;     //It will point to next block of the file
};

int readDiskBlock(int,int,char*);
int writeDiskBlock(int,int,char*);
int initDisk();
int createDisk();
int partition();
int vdcpto(char*,char*);
int vdcpfrom(char*,char*);
int vdls(char*);
int deleteFileFromDisk(char*);
void printErrorCode(int);
int disk_details();
int search(char*,int*,int*);
int testOperations();
void *myalloc(long unsigned int nbytes);
void myfree(void *freeptr);

