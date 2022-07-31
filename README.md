#File System -  

$make -> for compile
#Execution Command-	./all <input> output



-This code works on virtual disk (“disk.teasage”). We can access disk only through blocks. And blocksize must be taken from "disk_manual" . 
Whole disk will have same blocksize  at a time. So, if we want to change the blocksize in "disk_manual",we need to reset the disk with new blocksize.


-Blocksize should be atleast sizeof(struct fileInfo)bytes and/or multiple of sizeof(struct fileInfo)bytes to reduce internal fragmentation.

#Functionalities-
1) Store file on Virtual Disk 
2) Copy file from virtual disk to actual disk.
3) Delete file from Virtual Disk.
4) List all files resides on Virtual Disk.

# Approach:

The disk is divided into two parts.i.e data and metadata. I have reserved some blocks for metadata.
Metadata contains - 
a)fileInfo- name,size,starting block of file,endbytes(number of bytes in the last block of file).

b)DiskInfo- DiskName,number of files on the disk, number of free blocks on the disk.

c)Byte Array- Array to manage free blocks. If the byte is '0' ,block mapped to that byte on the virtual disk is 'free' and if the byte is '1' 
block mapped to that byte on the virtual disk is 'not free'.

d)linkedlist for managing file access.From file header we know the start block of the file and linkedlist will help us to read the next block of the file sequentially when -1 occurs we will know, end of file is reached. 

Data- Actual file Information.

#Some important Subroutines
1) vdcpto(char*filepath,char *filename)- It will take filepath of file on actual disk which we need to copy into the virtual disk and 
 filename-name of that file to store on virtual disk.

2)vdcpfrom(char *filepath,char *filename)- It will take filepath of file on actual disk where we need to copy file from the virtual disk and 
 filename-name of the file stored on virtual disk.

3)deleteFileFromDisk(char * filename)- filename-> name of the file in virtual disk which we want to delete.
After deleting the file its space also made free for other files.

4)vdls(char *buf) - buf contains file names and some information(12 bytes are reserved here- first 4 bytes for no. of files remaining to read,   second 4 bytes for next block number to read and next 4 bytes for" which file to read" after buffer is full).


#Automation- 
Automation is done for input and output .Also it checks if all operations are done properly or not.
There is "log.txt"  file which keeps track of recent file operations on disk. If there is any error occur in the operation we write error code in the last of that operation in the "log.txt". This file we can later use for testing.

For testing, we check if there is no error in the "log.txt" for the particular operation then we can proceed for testing-  
-We test if two files copy into/from disk properly or not. 
-For delete operation we can test by  searching for that file after "delete operation" and if file is found that means deletion is not done properly otherwise file got deleted properly.
If testing result is unsuccessful , we write -1 at last of operation in the "test_result" file. Otherwise we write 1 in the last of the operation in "test_result"

There is one problem in this approach as I am testing  after all operations done.The file which gets deleted, we don't have that file in the  disk now. So I have not tested operations related to that file in the disk. And I have written -1 in case of such files in the "test_result". This problem can be solved if we test as soon as operation is performed .

-We can later check "test_result" file manually to see if all file operations are done properly or not.

 









