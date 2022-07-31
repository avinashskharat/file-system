#include"driver.h"
int main()
{
	disk_details();
	FILE* lg= fopen("log.txt","w");
	if(lg==NULL)
	{
		lg=fopen("log.txt","w");
		if(lg==NULL)
			return -10;
	}
	long unsigned int n,i,j;
	int ch;
	char filepath[100],filenm[50];
	int err;
	int errnum= createDisk();
	partition();
        if(errnum==0)
        {
		initDisk();
        }
	
	int headerblock=0,headerpos=0;
	do
	{
		fprintf(ostream,"\n********MENU********\n1.copy file from AD to VD\n2.list all files of VD\n3.retrieve file from VD to AD\n4.delete file from VD\n5.exit()");
		fscanf(istream,"%d",&ch);
		switch(ch)
		{
			case 1: fprintf(ostream,"Enter Filepath: ");
				fscanf(istream,"%s",filepath);
				fprintf(ostream,"Enter Filename: ");
				fscanf(istream,"%s",filenm);
				err=search(filenm,&headerblock,&headerpos);
				if(err>=0)
				{
					err=-13;
					printErrorCode(err);
				}
				else
				{
					err=vdcpto(filepath,filenm);
					fprintf(lg,"%d\t%s\t%s\t%d\n",ch,filepath,filenm,err);
					printErrorCode(err);
					//fprintf(ostream,"\nSuccessful\n");
				}
				break;
			
			case 2: 
			{
			     	char*buf=(char*)myalloc(blocksize*sizeof(char));
       			     	struct diskInfo *diskb=(struct diskInfo*)myalloc(filehinoneblock*sizeof(struct diskInfo));
       				int vdfd=open("disk.teasage",O_RDONLY,00700);
        			if(vdfd==-1)
        			{
     		           		printErrorCode(-1);
					break;
        			}
       				if((readDiskBlock(vdfd,diskhead,(char*)diskb))==-1)
        			{
              			  
				  printErrorCode(-2);
				  break;
				}
				int totalfiles=diskb[0].numfiles;
				if(totalfiles==0)
				{
					myfree(diskb);
					close(vdfd);
					break;
				}
				myfree(diskb);
				close(vdfd);
				int *rest=(int*)&buf[0];
       				int*curb=(int*)&buf[4];
        			int*curpos=(int*)&buf[8];
        			*rest=totalfiles;
				*curb=diskhead+1;
				*curpos=0;
				err=vdls(buf);
				printErrorCode(err);
				totalfiles=*((int*)&buf[0]);	
				
				while(1)
				{
					totalfiles=*((int*)&buf[0]);
					int curblock=*((int*)&buf[4]);
					int curp=*((int*)&buf[8]);
					char* token=strtok((buf+12),"|"); 
					while(token!=NULL)
        				{
                       				printf("%s\n",token);
                        			token=strtok(NULL,"|");
        				}
					if(totalfiles==0)
						break;	
					memset(buf,' ',sizeof(buf));
					int *r=(int*)&buf[0];
        				int *c=(int*)&buf[4];
        				int *cp=(int*)&buf[8];
					*r=totalfiles;
					*c=curblock;
					*cp=curp;
					err=vdls(buf);
					printErrorCode(err);
				}
				myfree(buf);
				fprintf(lg,"%d\n",ch);
				break;
			}
			
                       
			case 3: fprintf(ostream,"Enter Filepath: ");
				fscanf(istream,"%s",filepath);
				fprintf(ostream,"Enter Filename: ");
				fscanf(istream,"%s",filenm);
				err=vdcpfrom(filepath,filenm);
				fprintf(lg,"%d\t%s\t%s\t%d\n",ch,filepath,filenm,err);
				printErrorCode(err);
				//fprintf(ostream,"Successful\n");
				break;
			
			case 4: fprintf(ostream,"Enter Filename");
				fscanf(istream,"%s",filenm);
				err=deleteFileFromDisk(filenm);
				fprintf(lg,"%d\t%s\t%d\n",ch,filenm,err);
				printErrorCode(err);
				break;
			
			case 5:	fprintf(lg,"%d\n",ch);
				break;
			
	        }
	
	}while(ch!=5);
	fclose(lg);
	if(testOperations()<0)
		fprintf(ostream,"ERROR IN TESTING\n");
	return 1;
}
