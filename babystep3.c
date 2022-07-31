#include"driver.h"

char nullbyte[] = "\0";
int head,freet,point,disksize,fileinfoend,content,diskblocks;

// disk_details() is used to read blocksize from disk_manual
int disk_details()
{
	char buf[100];
        int readbytes,x;
        int fd=open("disk_manual.txt",O_RDONLY);
        if(fd==-1)
        {
                return -1;
        }
        if(readbytes=read(fd,buf,sizeof(buf))==-1)
                return -1;
        sscanf(buf,"%d",&x);
        blocksize=x;        
	close(fd);
        return 0;
}

// printErrorCode() is used to print Error codes
void printErrorCode(int err)
{
	if(err<0)
	{
		fprintf(estream,"ERROR:%d\n",err);
	}
}
	
// readDiskBlock() is used to read particular block of disk 
int readDiskBlock(int fd,int blkno, char* buffptr)
{
	if(diskblocks<=blkno)
		return -11;
	if(lseek(fd,blkno*blocksize,SEEK_SET)==-1)
	{
		//perror("Lseek error:");
			return -1;
	}
	return read(fd,buffptr,blocksize);
}

// writeDiskBlock() is used to read particular block of disk
int writeDiskBlock(int fd,int blkno,char* buffptr)
{
	if(diskblocks<=blkno)
		return -11;

	if(lseek(fd,blkno*blocksize,SEEK_SET)==-1)
	{
		//perror("Lseek error:");
		return -1;
	}

	return (write(fd,buffptr,blocksize));
}

// createDisk() subroutine helps to create virtual disk(if not exist)
int createDisk()
{
	int fd;
	//printf("\n in create disk");
	off_t disksize1 = 10485760L - 1L;
	int vdfd = open("disk.teasage", O_WRONLY);
	if(vdfd==-1)
    	{
		fd=open("disk.teasage",O_WRONLY|O_CREAT,00700);
    	}
	else
	{
		if((disksize=lseek(vdfd,0,SEEK_END))==-1)
    		{
			close(vdfd);
        		return -2;
    		}
		close(vdfd);
		return -15;
    	}
    	if (fd ==-1) 
    	{
		return -1;
	}
    	if (write(fd, nullbyte, 1) != 1)
    	{
		return -3;
    	}
    	if (lseek(fd, disksize1, SEEK_SET) == -1)
    	{
		return -2;
    	}
   	if (write(fd, nullbyte, 1) != 1)
    	{
		return -3;
    	}
   	if((disksize=lseek(fd,0,SEEK_END))==-1)
    	{
		return -2;
    	}
    	close(fd);
   	return 0;
}
//initDisk() - This subroutine used to initialize the disk with proper information  in reserved blocks
int initDisk()
{

    int fd = open("disk.teasage", O_RDWR , 00700);
    if (fd ==-1) 
    {
	return -1;
    }
    int filepinoneblock=blocksize/sizeof(struct filepointers);
    int freepinoneblock=blocksize/sizeof(struct freetrack);
    filehinoneblock=blocksize/sizeof(struct fileInfo);
    struct diskInfo *diskb=(struct diskInfo*)myalloc(filehinoneblock*sizeof(struct diskInfo));
    for(int i=0;i<filehinoneblock;i++)
    {
	if(i==0)
	{
		strcpy((diskb+0)->diskname,"disk.teasage");
    		diskb[0].numfiles=0;
    		diskb[0].numfreeblocks=content;
	}
	else
	{
		strcpy((diskb+i)->diskname,"-1");
    		diskb[i].numfiles=-1;
    		diskb[i].numfreeblocks=-1;
        }
    }
    if((writeDiskBlock(fd,diskhead,(char*)diskb))==-1)
    {
	    myfree(diskb);
	    close(fd);
	    return -2;
    }
    myfree(diskb);

    for(int i=diskhead+1;i<fileinfoend;i++)
    {
        struct fileInfo*look=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));
        for(int j=0;j<filehinoneblock;j++)
        {
                strcpy((look+j)->filename,"-1"); 
                look[j].startno=-1;
                look[j].sizeinblocks=-1;
                look[j].endbytes=-1;
                //look[j].fileno=-1;
        }
        if((writeDiskBlock(fd,i,(char*)look))==-1)
	{
		myfree(look);
		close(fd);
		return -2;
	}
        myfree(look);
    }
    for(int i=0;i<point-1;i++)
    {
	    struct filepointers*filep=(struct filepointers*)myalloc(filepinoneblock*sizeof(struct filepointers));
	    for(int j=0;j<filepinoneblock;j++)
	    {
		    filep[j].next=-1;
	    }
	    if((writeDiskBlock(fd,i,(char*)filep))==-1)
            {
		myfree(filep);
		close(fd);
                return -2;
            }
            myfree(filep);
    }
    for(int i=point;i<diskhead;i++)
    {
            struct freetrack *freeb=(struct freetrack*)myalloc(freepinoneblock*sizeof(struct freetrack));
            for(int j=0;j<freepinoneblock;j++)
            {
                    freeb[j].isfree='0';
            }
            if((writeDiskBlock(fd,i,(char*)freeb))==-1)
            {
				myfree(freeb);
				close(fd);
				return -2;
            }
            myfree(freeb);
    }
    close(fd);

    return 0;
}

// partition() is used to partition the disk into metadata(byte array,file access list, file information headers) and content
//It calculates how many blocks should be allocated to each type of metadata depending on the blocksize and disksize
int partition()
{
	int filepinoneblock=blocksize/sizeof(struct filepointers);
    	int freepinoneblock=blocksize/sizeof(struct freetrack);
    	filehinoneblock=blocksize/sizeof(struct fileInfo);
	diskblocks= disksize/blocksize;
	freet=diskblocks/(freepinoneblock+1);
	point=(diskblocks-freet)/(filepinoneblock+1);
	head=(diskblocks-freet-point)/(filehinoneblock+1);
	freet=(diskblocks-head)/(freepinoneblock+1);
	if((diskblocks-head)%(freepinoneblock+1))
		freet++;
	point=(diskblocks-head-freet)/(filepinoneblock+1);
	if((diskblocks-head-freet)%(filepinoneblock+1))
		point++;
	head=(diskblocks-point-freet)/(filehinoneblock+1);
	if((diskblocks-point-freet)%(filehinoneblock+1))
		head++;
	head+=1 ;//for disk info one more header needed
        diskhead=freet+point;
	//printf("\ndiskhead=%d",diskhead);
	int fileinfostart=diskhead+1;
	fileinfoend=freet+point+head;
	content=diskblocks-fileinfoend;
	/*printf("\nfilesize=%d",disksize);
	printf("\nfilesizeinblocks=%d",diskblocks);
	printf("\nblocksforfreetrack=%d",freet);
	printf("\nblockforfilepointers=%d",point);
	printf("\nblocksforfileheaders=%d",head);
	printf("\ninfo=%d",fileinfoend);
	printf("\ncontent=%d",content);*/
} 

// vdcpto() - This subroutine copies file from actual disk to virtual disk
int vdcpto(char* filepath,char* filename)
{
	int filepinoneblock=blocksize/sizeof(struct filepointers);
	int freepinoneblock=blocksize/sizeof(struct freetrack);
	filehinoneblock=blocksize/sizeof(struct fileInfo);
	int readbytes,endbytes,filesize,filesizeinblocks,blockptr,cur,blocktoread;
  	int i,headerpos,headerblock,j;
	char buf[blocksize];
	int vdfd=open("disk.teasage",O_RDWR,00700);  //open vdfd in Readwrite mode
	if(vdfd==-1)
	{
		return -1;
	}
	int adfd=open(filepath,O_RDONLY,00700);  //open ACtual Disk File
	if(adfd==-1)
	{
		close(vdfd);
		return -4;
	}
	if((filesize=lseek(adfd,0,SEEK_END))==-1)//Calculate filesize
	{
		close(vdfd);
		close(adfd);
		return -5;
	}
	if((lseek(adfd,0,SEEK_SET))==-1)
	{
		close(vdfd);
		close(adfd);
		return -5;
	}
	filesizeinblocks=filesize/blocksize; //filesize in blocks
	if(filesize%blocksize)
		filesizeinblocks++;//if extra bytes then one block increases*/
	//printf("\nfilesizeinblocks= %d",filesizeinblocks);
	struct diskInfo*diskb=(struct diskInfo*)myalloc(filehinoneblock*sizeof(struct diskInfo));
  	if((readDiskBlock(vdfd,diskhead,(char*)diskb))==-1) //reading DIsk Info as it is stored in block 178
  	{
		myfree(diskb);
		close(vdfd);
		close(adfd);
    		return -2;
  	}
  	if(diskb[0].numfreeblocks<filesizeinblocks)  //check if enough space available to copy
  	{
		//perror("\n Not enough space to copy")
		myfree(diskb);
		close(vdfd);
		close(adfd);
    		return -6;
  	}
  	// you will have fileheaders for its info (noofFiles=noofHeaders)
	diskb[0].numfiles+=1;
	diskb[0].numfreeblocks-=filesizeinblocks;
	if((writeDiskBlock(vdfd,diskhead,(char*)diskb))==-1)
	{
		myfree(diskb);
		close(vdfd);
		close(adfd);
		return -2;
	}
  	myfree(diskb);
	//struct fileInfo*look=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));
  	int flag=0;
  	for(i=diskhead+1;i<fileinfoend;i++)    //179-2434 we have FILE Headers
  	{
		struct fileInfo*look=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));
    		if((readDiskBlock(vdfd,i,(char*)look))==-1)
		{
			myfree(look);
			close(vdfd);
			close(adfd);
      			return -2;
		}
    		for(j=0;j<filehinoneblock;j++)
    		{
      			if((strcmp((look+j)->filename,"-1"))==0)
      			{
				
        			headerpos=j;
        			headerblock=i;
				if((writeDiskBlock(vdfd,i,(char*)look))==-1)
				{
					myfree(look);
					close(vdfd);
					close(adfd);
					return -2;
				}
        			flag=1;
        			break;
			}
    		}
    		if(flag==1)
		{
			myfree(look);
      			break;
		}
  	}
	struct fileInfo*look1=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo)); // writing header info
	if((readDiskBlock(vdfd,headerblock,(char*)look1))==-1)
	{
		myfree(look1);
		close(vdfd);
		close(adfd);
		return -2;
	}
	strcpy((look1+headerpos)->filename,filename);
	look1[headerpos].sizeinblocks=filesizeinblocks;
	if((writeDiskBlock(vdfd,headerblock,(char*)look1))==-1)
	{
		myfree(look1);
		close(vdfd);
		close(adfd);
		return -2;
	}
	myfree(look1);
	if((readbytes=read(adfd,buf,blocksize))==-1)                     //read first block of actual  disk
	{
		close(vdfd);
		close(adfd);
		return -7;
	
	}
	
	int count=0;
	flag=0;
	for(i=point;i<diskhead;i++) //freeblocks track
 	{
		struct freetrack*block=(struct freetrack*)myalloc(blocksize*sizeof(struct freetrack));
		struct fileInfo*look1=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));

    		struct filepointers*fileptr1=(struct filepointers*)myalloc(filepinoneblock*sizeof(struct filepointers));
   	 	if((readDiskBlock(vdfd,i,(char*)block))==-1)
		{
			myfree(block);
			myfree(look1);
			myfree(fileptr1);
			close(vdfd);
			close(adfd);
			return -2;
		}
    		for(j=0;j<blocksize;j++)
    		{
      			if(block[j].isfree=='0')
      			{
				block[j].isfree='1';
				int freeptr=(i-point)*blocksize+j+fileinfoend;  //This block in vd is free for writing
				count++;
        			if(count==1)//1st block of file
        			{
            				if((readDiskBlock(vdfd,headerblock,(char*)look1))==-1)
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(vdfd);
						close(adfd);
						return -2;
					}
          				
					look1[headerpos].startno=freeptr;
            				
					if((writeDiskBlock(vdfd,headerblock,(char*)look1))==-1)
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(vdfd);
						close(adfd);
						return -2;
					}
				

				}
        			if(count>1)
        			{
					blocktoread=(cur-fileinfoend)/filepinoneblock;
          				blockptr=(cur-fileinfoend)%filepinoneblock;
         				if((readDiskBlock(vdfd,blocktoread,(char*)fileptr1))==-1)
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(vdfd);
						close(adfd);
						return -2;
					}
          				fileptr1[blockptr].next=freeptr;
					if((writeDiskBlock(vdfd,blocktoread,(char*)fileptr1))==-1)
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(vdfd);
						close(adfd);
						return -2;
					}

				}

        			if(readbytes!=0)
        			{
         	 			if((writeDiskBlock(vdfd,freeptr,(char*)buf))==-1) 
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(vdfd);
						close(adfd);
						return -2;
					}	
          				endbytes=readbytes;
          				if((readbytes=read(adfd,buf,blocksize))==-1)
          				{
            					myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(adfd);
            					close(vdfd);
						return -7;
          				}
        			}
				cur=freeptr;
				if(count==filesizeinblocks)
				{
					blocktoread=(cur-fileinfoend)/filepinoneblock;
                        		blockptr=(cur-fileinfoend)%filepinoneblock;
                        		if((readDiskBlock(vdfd,blocktoread,(char*)fileptr1))==-1)
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(adfd);
            					close(vdfd);
                            			return -2;
					}
					
                       			fileptr1[blockptr].next=-1;
					if((writeDiskBlock(vdfd,blocktoread,(char*)fileptr1))==-1)
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(adfd);
            					close(vdfd);
						return -2;
					}
					
					if((readDiskBlock(vdfd,headerblock,(char*)look1))==-1)
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(adfd);
            					close(vdfd);
					}
                        		look1[headerpos].endbytes=endbytes;
					
              				if((writeDiskBlock(vdfd,headerblock,(char*)look1))==-1)
					{
						//perror("freealloc writedisk error");
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(adfd);
            					close(vdfd);
						return -2;
					}
					if((readDiskBlock(vdfd,headerblock,(char*)look1))==-1)
					{
						myfree(block);
						myfree(look1);
						myfree(fileptr1);
						close(adfd);
            					close(vdfd);
						return -2;
					}
					/*printf("\nlook1[headerpos].startno=%d",look1[headerpos].startno);
					printf("\nlook1[headerpos].filename=%s",look1[headerpos].filename);
					printf("\nlook1[headerpos].sizeinblocks=%d",look1[headerpos].sizeinblocks);
					printf("\nlook1[headerpos].endbytes=%d",look1[headerpos].endbytes);
					printf("\nheaderblock=%d\nheaderpos=%d",headerblock,headerpos);*/
					flag=1;
					break;
					
				}
			}
		}
		if((writeDiskBlock(vdfd,i,(char*)block))==-1)
		{
			myfree(block);
			myfree(look1);
			myfree(fileptr1);
			close(adfd);
            		close(vdfd);
			return -2;
		}
		if(flag==1)
			break;

		myfree(block);
		myfree(look1);
		myfree(fileptr1);
	}
	
	close(vdfd);
	close(adfd);
	return 0;

}

// vdls() - This subroutine list all files stored on the virtual disk
int vdls(char*buf)
{
	int filepinoneblock=blocksize/sizeof(struct filepointers);
        int freepinoneblock=blocksize/sizeof(struct freetrack);
        filehinoneblock=blocksize/sizeof(struct fileInfo);
        size_t bufsize= sizeof(buf);


	int rem=*((int*)&buf[0]);
	int cur=*((int*)&buf[4]);
	int curfile=*((int*)&buf[8]);

	int count=0,flag=0,m=12,i,j;
	int vdfd=open("disk.teasage",O_RDONLY,00700);
        if(vdfd==-1)
        {
               return -1;
        }
	struct fileInfo fi;
	int filename_size=sizeof(fi.filename);
	
	for(i=cur;i<fileinfoend;i++)
	{
		struct fileInfo*look=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));
		if((readDiskBlock(vdfd,i,(char*)look))==-1)
		{
			//perror("\nread disk error");
			myfree(look);
			close(vdfd);
			return -2;
		}
		
		for(j=curfile;j<filehinoneblock;j++)
		{
			if((strcmp((look+j)->filename,"-1"))!=0)
			{

				if((m+filename_size)>blocksize)             
                                {
                                        flag=1;
                                        curfile=j;
                                        break;
                                }
				strcat((look+j)->filename,"|");
				strcpy((buf+m),(look+j)->filename);
                		m+=strlen((look+j)->filename);
				count++;

                		if(count==rem)
				{
					flag=2;
					break;
				}
			}
			if(flag!=1)
				curfile=0;
		}
		myfree(look);
		if(flag==1 || flag==2)
			break;
		cur++;
	}
	
	rem-=count;

	int *cblock=(int*)&buf[4];
	int *remfile=(int*)&buf[0];
	int *curpos=(int*)&buf[8];
	*cblock=cur;
	*remfile=rem;
	*curpos=curfile;
	close(vdfd);
	return 0;

}
//search() - This subroutine search for the file in virtual disk.
//here headerblock and headerpos is fileheader block and fileader position in that block respectively.
int search(char* filename,int*headerblock,int*headerpos)
{
	int filepinoneblock=blocksize/sizeof(struct filepointers);
        int freepinoneblock=blocksize/sizeof(struct freetrack);
        filehinoneblock=blocksize/sizeof(struct fileInfo);
	
	int vdfd=open("disk.teasage",O_RDONLY,00700);
        if(vdfd==-1)
        {
                return -1;
        }
       	struct diskInfo* block=(struct diskInfo*)myalloc(filehinoneblock*sizeof(struct diskInfo));
        if((readDiskBlock(vdfd,diskhead,(char*)block))==-1)
        {
                myfree(block);
		close(vdfd);
                return -2;
        }
	int totalheaders=block[0].numfiles;
        int blockstoread=block[0].numfiles/filehinoneblock;
        if((block[0].numfiles)%filehinoneblock)
                blockstoread++;
	myfree(block);
        int flag=0;
	struct fileInfo*look=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));
	
	for(int i=diskhead+1;i<fileinfoend;i++)
        {
                //struct fileInfo*look=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));

                if((readDiskBlock(vdfd,i,(char*)look))==-1)
                {
                        myfree(look);
			close(vdfd);
                        return -2;
                }
                for(int j=0;j<filehinoneblock;j++)
                {
                        if((strcmp((look+j)->filename,filename))==0)
                        {
                               // printf("\nFIle found");
				*headerblock=i;
                                *headerpos=j;
                                flag=1;
                                break;
                        }
                }
                if(flag==1)
                {
                        myfree(look);
			close(vdfd);
			return 1;
                        
                }
        }
	myfree(look);
	close(vdfd);
        if(flag==0)
        {
                //perror("file not found");
                return -9;
        }
	return 1;

}

// vdcpfrom() - This subroutine copies file from virtual disk to actual disk at designated file path
int vdcpfrom(char*filepath,char*filename)
{
	int headerblock=0,headerpos=0;
	int error_num=search(filename,&headerblock,&headerpos);
        if(error_num<0)
		return error_num;

	int filepinoneblock=blocksize/sizeof(struct filepointers);
        int freepinoneblock=blocksize/sizeof(struct freetrack);
        filehinoneblock=blocksize/sizeof(struct fileInfo);
         
	int vdfd=open("disk.teasage",O_RDONLY,00700);
	if(vdfd==-1)
	{
		return -1;
	}
	int adfd=open(filepath,O_RDWR |O_CREAT,00700);
        if(adfd==-1)
        {
                close(vdfd);
                return -4;
        }
	struct fileInfo*look1=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));
	if((readDiskBlock(vdfd,headerblock,(char*)look1))==-1)
        {
		myfree(look1);
		close(vdfd);
		close(adfd);
        	return -2;
        }
	int startblock=look1[headerpos].startno;
	int filesizeinblocks=look1[headerpos].sizeinblocks;
	int lastbytes=look1[headerpos].endbytes;
	myfree(look1);
	int blocktoread,blockptr;
	char buf[blocksize];
	struct filepointers*fileptr=(struct filepointers*)myalloc(filepinoneblock*sizeof(struct filepointers));

	for(int i=0;i<filesizeinblocks-1;i++)
	{
		if((readDiskBlock(vdfd,startblock,(char*)buf))==-1)
		{
			myfree(fileptr);
			close(vdfd);
			close(adfd);
			return -2;
		}
		if((write(adfd,buf,blocksize))==-1)
		{
			myfree(fileptr);
			close(vdfd);
			close(adfd);
			return -8;
		}
		blocktoread=(startblock-fileinfoend)/filepinoneblock;
		blockptr=(startblock-fileinfoend)%filepinoneblock;
		if((readDiskBlock(vdfd,blocktoread,(char*)fileptr))==-1)
                {
			
			myfree(fileptr);
			close(vdfd);
			close(adfd);
                        return -2;
                }
		startblock=fileptr[blockptr].next;
		
	}
	if((readDiskBlock(vdfd,startblock,(char*)buf))==-1)
	{
	
		myfree(fileptr);
		close(vdfd);
		close(adfd);
		return -2;
	}
	if((write(adfd,buf,lastbytes))==-1)
	{
		myfree(fileptr);
		close(vdfd);
		close(adfd);
		return -8;
	}
	
	myfree(fileptr);
	close(vdfd);
	close(adfd);
        return 0;
}
// deleteFileFromDisk() This subroutine is used for deleting the file from virtual disk
int deleteFileFromDisk(char*filename)
{
	int startblock,filesizeinblocks;
	int filepinoneblock=blocksize/sizeof(struct filepointers);
        int freepinoneblock=blocksize/sizeof(struct freetrack);
        filehinoneblock=blocksize/sizeof(struct fileInfo);

	int found=0;
	//struct diskInfo* diskb=(struct diskInfo*)myalloc(8*sizeof(struct diskInfo));*/
	int headerblock=0,headerpos=0;
	int errnum=search(filename,&headerblock,&headerpos);
	if(errnum<0)
	{
		return errnum;
	}
        int vdfd=open("disk.teasage",O_RDWR,00700);
        if(vdfd==-1)
        {
               
                return -1;
        }
	//Since file is found on the disk we can delete it..
	
	struct fileInfo *look=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));
	if((readDiskBlock(vdfd,headerblock,(char*)look))==-1)
	{
		myfree(look);
		close(vdfd);
		return -2;
	}
	startblock=look[headerpos].startno;
	filesizeinblocks=look[headerpos].sizeinblocks;
	strcpy((look+headerpos)->filename,"-1");
	if((writeDiskBlock(vdfd,headerblock,(char*)look))==-1)
        {
		myfree(look);
		close(vdfd);
        	return -2;
        }
	myfree(look);
	struct diskInfo* diskb=(struct diskInfo*)myalloc(filehinoneblock*sizeof(struct diskInfo));
	
	if((readDiskBlock(vdfd,diskhead,(char*)diskb))==-1)
	{
		myfree(diskb);
		close(vdfd);
		return -2;
	}
	//printf("\nnumfreeblocks before deletion =%d",diskb[0].numfreeblocks);
	diskb[0].numfiles-=1;
	diskb[0].numfreeblocks+=filesizeinblocks;
	//printf("\n before writing numfreeblocks= %d",diskb[0].numfreeblocks);
	if((writeDiskBlock(vdfd,diskhead,(char*)diskb))==-1)
	{
		myfree(diskb);
		close(vdfd);
		return -2;
	}
	if((readDiskBlock(vdfd,diskhead,(char*)diskb))==-1)
	{
		myfree(diskb);
		close(vdfd);
		return -2;
	}
	
	myfree(diskb);
	struct filepointers* fileptr=(struct filepointers*)myalloc(filepinoneblock*sizeof(struct filepointers));
	struct freetrack* freeb=(struct freetrack*)myalloc(freepinoneblock*sizeof(struct freetrack));
	int curblock=(startblock-fileinfoend)/filepinoneblock;  //in filepointers startblock is located at curblock;
	int curpos=(startblock-fileinfoend)%filepinoneblock;    // curpos is exact location of startblock in filepointers section
	if((readDiskBlock(vdfd,curblock,(char*)fileptr))==-1)
	{
		myfree(fileptr);
		myfree(freeb);
		close(vdfd);
		return -2;
	}
	int curfreeb=(startblock-fileinfoend)/blocksize+point;
	int curfreepos=(startblock-fileinfoend)%blocksize;
	if((readDiskBlock(vdfd,curfreeb,(char*)freeb))==-1)
	{
		myfree(fileptr);
		myfree(freeb);
		close(vdfd);
		return -2;
	}
	freeb[curfreepos].isfree='0';
	if((writeDiskBlock(vdfd,curfreeb,(char*)freeb))==-1)
	{
		myfree(fileptr);
		myfree(freeb);
		close(vdfd);
		return -2;
	}
	
	while(fileptr[curpos].next!=-1)
	{
		startblock=fileptr[curpos].next;
		curfreeb=(startblock-fileinfoend)/blocksize+point;
        	curfreepos=(startblock-fileinfoend)%blocksize;
        	if((readDiskBlock(vdfd,curfreeb,(char*)freeb))==-1)
		{
			myfree(fileptr);
			myfree(freeb);
			close(vdfd);
			return -2;
		}
        	freeb[curfreepos].isfree='0';
        	if((writeDiskBlock(vdfd,curfreeb,(char*)freeb))==-1)
		{
			myfree(fileptr);
			myfree(freeb);
			close(vdfd);
			return -2;
		}
		curblock=(startblock-fileinfoend)/filepinoneblock;  
        	curpos=(startblock-fileinfoend)%filepinoneblock; 
		if((readDiskBlock(vdfd,curblock,(char*)fileptr))==-1)
		{
			myfree(fileptr);
			myfree(freeb);
			close(vdfd);
			return -2;
		}
	}
	myfree(fileptr);
	myfree(freeb);
	close(vdfd);
	return 0;
}

// fileCompare() - This subroutine is used to test if two files i.e file copied from/into virtual disk from/into actual disk are same or not 
int fileCompare(char * adfile,char* vdfile)
{
	int filesize;
	int filepinoneblock=blocksize/sizeof(struct filepointers);
        int freepinoneblock=blocksize/sizeof(struct freetrack);
        filehinoneblock=blocksize/sizeof(struct fileInfo);

	int vdfd=open("disk.teasage",O_RDONLY);
	if(vdfd==-1)
		return -1;
	int adfd=open(adfile,O_RDONLY);
        if(adfd==-1)
	{
		close(vdfd);
		return -4;
	}
	char* buf= (char*)myalloc(blocksize*sizeof(char));
	int headerblock=0,headerpos=0,readbytes;
	int errnum=search(vdfile,&headerblock,&headerpos);
	if(errnum<0)
	{
		myfree(buf);
		close(vdfd);
        	close(adfd);
		return -9;
	}
	// first check if size of both the files are same or not
	struct fileInfo*look1=(struct fileInfo*)myalloc(filehinoneblock*sizeof(struct fileInfo));
        if((readDiskBlock(vdfd,headerblock,(char*)look1))==-1)
        {
                
		myfree(buf);
		myfree(look1);
		close(vdfd);
        	close(adfd);		
                return -2;
        }
        int startblock=look1[headerpos].startno;
        int filesizeinblocks=look1[headerpos].sizeinblocks;
        int lastbytes=look1[headerpos].endbytes;
	if((filesize=lseek(adfd,0,SEEK_END))==-1)//Calculate filesize
        {
                //perror("\nlseek error AD");
                //exit(1);
		myfree(buf);
		myfree(look1);
		close(vdfd);
        	close(adfd);
                return -5;
        }
        if((lseek(adfd,0,SEEK_SET))==-1)
        {
		myfree(buf);
		myfree(look1);
		close(vdfd);
        	close(adfd);
                return -5;
        }
	if(filesize!=(filesizeinblocks-1)*blocksize+lastbytes) // size not same
	{
		myfree(buf);
		myfree(look1);
		close(vdfd);
        	close(adfd);
		return -12;
	}
	myfree(buf);
	myfree(look1);
	int blocktoread,blockptr;
        char vdbuf[blocksize];
	char adbuf[blocksize];
        struct filepointers*fileptr=(struct filepointers*)myalloc(filepinoneblock*sizeof(struct filepointers));

        for(int i=0;i<filesizeinblocks-1;i++)
        {
                if((readDiskBlock(vdfd,startblock,(char*)vdbuf))==-1)
                {
                        
			myfree(fileptr);
			close(adfd);
			close(vdfd);
                        return -2;
                }
                
		readbytes=read(adfd,adbuf,blocksize);
		for(int k=0;k<blocksize;k++)
		{
			if(adbuf[k]!=vdbuf[k])
			{
				myfree(fileptr);
				close(adfd);
				close(vdfd);
				return -12;
			}
		}

                blocktoread=(startblock-fileinfoend)/filepinoneblock;
                blockptr=(startblock-fileinfoend)%filepinoneblock;
                if((readDiskBlock(vdfd,blocktoread,(char*)fileptr))==-1)
                {
                        //perror("readdiskerror");
                        //exit(1);
			myfree(fileptr);
			close(adfd);
			close(vdfd);
                        return -2;
                }
                startblock=fileptr[blockptr].next;
                //printf("\nstartblock=%d",startblock);
        }
	if((readDiskBlock(vdfd,startblock,(char*)vdbuf))==-1)
        {
                myfree(fileptr);
		close(adfd);
		close(vdfd);
                return -2;
        }
        
	readbytes=read(adfd,adbuf,blocksize);

        for(int k=0;k<lastbytes;k++)
        {
        	if(adbuf[k]!=vdbuf[k])
		{
			myfree(fileptr);
			close(adfd);
			close(vdfd);
                	return -12;
		}
        }

        //free(look1);
        myfree(fileptr);

        close(vdfd);
        close(adfd);

        return 0;
}

void testHelper(FILE* tr,char* str)
{
	//printf("hi in test1");
	int n=strlen(str);
	int headerblock=0,headerpos=0;
	char *token[]={NULL,NULL,NULL,NULL};
	token[0]=strtok(str,"\t");
	int i=0;
	while(token[i]!=NULL)
	{
		i++;
		if(i==4)
			break;
		token[i]=strtok(NULL,"\t");
	}
	int choice=atoi(token[0]);
	//printf("\nchoice= %d",choice);
	if(token[3]!=NULL)
	{
		if(i=atoi(token[3])<0) 	// No need to check if there is error in the operation
		{
			return;
		}
	}
	switch(choice)
	{
		case 1:	fprintf(tr,"vdcpto\t%s\t%s\t",token[1],token[2]); 		//vdcpto
			if(fileCompare(token[1],token[2])>=0)
				fprintf(tr,"1\n");	//both files are same 
			else
				fprintf(tr,"-1\n");	//files are not same
			break;
		case 3: fprintf(tr,"vdcpfrom\t%s\t%s\t",token[1],token[2]);  		//vdcpfrom
			if(fileCompare(token[1],token[2])>=0)
				fprintf(tr,"1\n");	//both files are same
			else
				fprintf(tr,"-1\n");	//files are not same
			break;
		case 4: fprintf(tr,"deleteFileFromDisk\t%s\t",token[1]);		//deleteFileFromDisk
			if((i=search(token[1],&headerblock,&headerpos))>=0)
				fprintf(tr,"-1\n");	//file is not deleted
			else
				fprintf(tr,"1\n");	//file deleted successfully
			break;
		case 5: fprintf(tr,"EXIT\n");		//EXIT				//Exit
			break;
	
	}
}
//testOperations() - This subroutine is used to test the operations performed on filesystem are correct or not
int testOperations()
{
	FILE* lg=fopen("log.txt","r");
	FILE* tr=fopen("test_result","w");
	if(lg==NULL || tr==NULL)
	{
		return -13;
	}
	char ch;
	int i=0;
	char*buf = (char*)myalloc(sizeof(char)*120);
	if(buf==NULL)
	{
		fclose(lg);
		fclose(tr);
		return -14;
	}
	while((ch=getc(lg))!=EOF)
	{
		
		if(ch=='\n')
		{
			buf[i]='\0';
			testHelper(tr,buf);
			i=0;	
		}
		else
		{
			buf[i]=ch;
			i++;
		}
	}
	fclose(lg);
	fclose(tr);
	myfree(buf);
	return 0;
}

		
	      	
