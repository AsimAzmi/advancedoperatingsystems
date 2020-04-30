#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#ifdef FS
#include <fs.h>



static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
#define MAXSIZE 1536

struct filetable oft[NUM_FD]; // open file table
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2



int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

/* write inode indicated by pointer to device */
int fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
/* create file system on device; write file system block and block bitmask to
 * device */
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

/* print information related to inodes*/
void fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) { // print block bitmask
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}



int fs_open(char *filename, int flags) 
{
  if(!(flags == O_WRONLY || flags == O_RDONLY || flags == O_RDWR))
  {
    printf("\n fs_open : Incorrect flag entered.");
    return SYSERR;
  }
  
  int i;
  for(i=0; i<fsd.root_dir.numentries; i++)
  {
    if(strcmp(fsd.root_dir.entry[i].name, filename)==0)
    {
      break;
    }
  }

  if(i >= fsd.root_dir.numentries){
    printf("\n fs_open : File not present in FSD (FileSystem)");
    return SYSERR;  
  }

  int j;
  int index_o_file = -1;
  int flag = 0;
  for(j = 0; j< NUM_FD; j++)
  {
    if(oft[j].in.id == fsd.root_dir.entry[i].inode_num)
    {
      index_o_file = j;
      if(oft[j].state == FSTATE_CLOSED)
      {
        index_o_file = j;
        break;
      }
      else if(oft[j].state == FSTATE_OPEN)
      {
        printf("\n fs_open: File already opened.");
        flag = 1;
        break; //return SYSERR;
      }
    }

    if(oft[j].state == FSTATE_CLOSED && index_o_file == -1){
      index_o_file = j;
    }
  }


  if (flag == 1)
    {
      printf("\n file already opened");
      return ;
    }

  if(index_o_file == -1)
  {
    printf("\n open_file : No space in OFT. Open file limit exceeded");
    return SYSERR;
  }

  int status;
  struct inode in;
  if((status = fs_get_inode_by_num(0, fsd.root_dir.entry[i].inode_num, &in)) == SYSERR){
    printf("\n fs_open : Failed to retrieve inode from FSD");;
    return SYSERR;
  }

  /* make an openfiletable entry */
  oft[index_o_file].state = FSTATE_OPEN;
  oft[index_o_file].fileptr = 0;
  oft[index_o_file].de = &fsd.root_dir.entry[i];
  oft[index_o_file].in = in;
  oft[index_o_file].flag = flags;
  return index_o_file;
}

int fs_close(int fd) 
{
  if(fd < 0 || fd >= NUM_FD){
    printf("\n Invalid file descriptor.");
    return SYSERR;
  }
 
  if(oft[fd].state == FSTATE_OPEN){
    oft[fd].state = FSTATE_CLOSED;
    oft[fd].fileptr = 0;
    return OK;
  }
  else
  {
    printf("\n fs_close: closing a file which is not open");
     return SYSERR;
  }
}

int fs_create(char *filename, int mode)
{
  int status;
  int i;
  struct inode in;


  if(mode != O_CREAT)
  {
    printf("fs_create : Mode check failed\n");
    return SYSERR;
  }
  

  if(strlen(filename) == 0 || strlen(filename) > FILENAMELEN)
  {
    printf("fs_create : Filename check failed\n");
    return SYSERR;
  }

  for(i=0; i<fsd.root_dir.numentries; i++)
  {
    if(strcmp(fsd.root_dir.entry[i].name, filename)==0)
    {
      printf("\n fs_create : File already present.");
      return SYSERR;
    }
  }

  if(fsd.inodes_used >= fsd.ninodes){
    printf("\n fs_create : No inodes available");
    return SYSERR;
  }
    
  for(i = 0; i < fsd.ninodes; i++){
    if(inodes_state[i] != USED){
      break;
    }
  }
  

  
  if((status = fs_get_inode_by_num(0, i, &in))==SYSERR){
    printf("\n fs_create: Inode fs_get_inode_by_num failed");
    return SYSERR;
  }

  /* fill up the inode information */ 
  in.id = fsd.inodes_used;
  in.type = INODE_TYPE_FILE;
  in.nlink = 1;
  in.device = 0;
  in.size = 0;

  /* writeback to the memory */
  if((status = fs_put_inode_by_num(0, i, &in))==SYSERR){
    printf("\n fs_create : fs_put_inode_by_num() failed");
    return SYSERR;
  }
  /* update the inodes_state array with ith index as used. */
  printf("\n Obtained inode. \n");
  inodes_state[i] = USED;
  
  /* fill up the entry structure of the root_dir */
  fsd.root_dir.entry[fsd.root_dir.numentries].inode_num = i;
  strcpy(fsd.root_dir.entry[fsd.root_dir.numentries].name, filename);
    
  /* file is created */
  fsd.inodes_used++;
  oft[fsd.inodes_used].state = FSTATE_OPEN;
  oft[fsd.inodes_used].fileptr = 0;
  oft[fsd.inodes_used].in = in;
  oft[fsd.inodes_used].de = &fsd.root_dir.entry[fsd.root_dir.numentries];
  oft[fsd.inodes_used].flag = O_RDWR;
  

  /* increment inodes_used and the numentries count */
  //fsd.inodes_used++;
  fsd.root_dir.numentries++;
  
  // Returning file table index
  return fsd.inodes_used;
}


int fs_seek(int fd, int offset) 
{
  
  if(fd<0 || fd>NUM_FD){
    printf("\n fs_seek : Invalid file descriptor.");
    return SYSERR;
  }
 
  if(oft[fd].state != FSTATE_OPEN){
    printf("\n fs_seek : failed: file is noteopen");
    return SYSERR;
  }

  oft[fd].fileptr =  oft[fd].fileptr + offset;
  return oft[fd].fileptr;
}

int fs_read(int fd, void *buf, int nbytes) {
  
  if(fd<0 || fd>NUM_FD){
    printf("\n fs_read : Invalid file descriptor.");
 
    return SYSERR;
  }

  if(oft[fd].state != FSTATE_OPEN || (oft[fd].flag != O_RDWR && oft[fd].flag != O_RDONLY))
  {    
    printf("\n fs_read:  File not open or opened in Write mode.");
    return SYSERR;
  }
  
  int inode_number, status;
  struct inode in_temp;

  inode_number = oft[fd].de->inode_num;
 
  if((status = fs_get_inode_by_num(0, inode_number, &in_temp)) == SYSERR){
    printf("\n fs_write : Failed to retrieve inode from FSD");;
    return SYSERR;
  }
  printf("\n fs_read: found inode %d size %d",in_temp.id, in_temp.size);


  /*if(oft[fd].in.size == 0){
    printf("\n fs_read : file is empty");
    return SYSERR;
  }*/
  if (in_temp.size == 0)
  {
    printf("\n fs_read : empty file ");
    return OK;
  }

  //nbytes += oft[fd].fileptr;
  int no_of_blocks=0;
  no_of_blocks = nbytes / MDEV_BLOCK_SIZE;
  if(nbytes % MDEV_BLOCK_SIZE !=0)
  {
    no_of_blocks++;
  }
 
  //int i, j;
  //i = (oft[fd].fileptr / MDEV_BLOCK_SIZE);
  //i = 0;
  
  //printf("\n reading starting from %d ", oft[fd].fileptr % MDEV_BLOCK_SIZE);

  memset(buf, NULL, MAXSIZE);
  int temp=0;
  int data_bytes=0;
  int offset = 0;

  for (int i = 0; i < in_temp.size; i++)
  {
    printf("\n retrieve block %d", in_temp.blocks[i]);
    //offset = oft[fd].fileptr % MDEV_BLOCK_SIZE;
    printf("\n offset %d", offset);
    //memset(block_cache, NULL, MDEV_BLOCK_SIZE+1);
    if (bs_bread(0, in_temp.blocks[i], offset, block_cache, MDEV_BLOCK_SIZE) == SYSERR)
    {
      printf("\n fs_read : read file failed");
      return SYSERR;
    }
    strcpy((buf+temp), block_cache);
    printf("\n%s", block_cache);
    printf("\n**************************************");
    temp = strlen(block_cache);
    data_bytes += temp;
    oft[fd].fileptr = data_bytes;
    printf("\n data_bytes %d:" ,data_bytes);
    //printf("%d blockupdated : %d bytes ",i, data_bytes );
    //offset=0;
  }

  //return oft[fd].fileptr;
  return nbytes;
}

int fs_write(int fd, void *buf, int nbytes) 
{
  /* Check FileDescriptor index is valid */
  if(fd<0 || fd>NUM_FD)
  {
    printf("\n fs_write :  Not a valid file descriptor.");
    return SYSERR;
  }
  
  /* Check if file is opened and have access to write*/
  if(oft[fd].state != FSTATE_OPEN )
  {
    printf("\n fs_write: File note open");
    return SYSERR;
  }

  if (oft[fd].flag != O_RDWR && oft[fd].flag != O_WRONLY)
  {
     printf("\n fs_write: File is opened in read only mode");
    return SYSERR;
  }
  
  struct inode in_temp;

  int status;
  int inode_number;
  inode_number = oft[fd].de->inode_num;

  if((status = fs_get_inode_by_num(0, inode_number, &in_temp)) == SYSERR){
    printf("\n fs_write : Failed to retrieve inode from FSD");;
    return SYSERR;
  }

  //kprintf("\n fs_write: retrieved inode %d", in_temp.nlink);
  
  int blocks_count = 0;
  blocks_count = nbytes / MDEV_BLOCK_SIZE;
  if(nbytes % MDEV_BLOCK_SIZE !=0)
  {
    blocks_count++;
  }
  
  int data_in_bytes;
  data_in_bytes = nbytes;
  int i = 0;
  int temp_fptr = oft[fd].fileptr;
  
  for ( int j = 3; j < 512; j++)
  {
    if (fs_getmaskbit(j) ==0)
    {
     // printf("\n block found");

      in_temp.blocks[i] = j;
      printf("\n entering block %d", in_temp.blocks[i]);
      //printf("\n writing on block %d", j);
      
      /* get the minimum bytes to write */
      int minBytes = 0;
      if ( MDEV_BLOCK_SIZE < data_in_bytes)
        minBytes = MDEV_BLOCK_SIZE;
      else
        minBytes = data_in_bytes;
      
      /* copy the data into block_cache */
      memcpy(block_cache, buf, MDEV_BLOCK_SIZE);
      
      /* write the data into the data block */
      //if(writeBlock(0, j, 0, block_cache, MDEV_BLOCK_SIZE) == SYSERR){
      if(bs_bwrite(0, j, 0, block_cache, MDEV_BLOCK_SIZE) == SYSERR)
      {  
        printf("\n fs_write : write failed on block %d.",j);
        return SYSERR;
      }
      //oft[fd].fileptr = oft[fd].fileptr + 
      buf = (char*) buf + minBytes;
      printf("\n%s",block_cache);
      printf("\n****************************");
      data_in_bytes = data_in_bytes - minBytes;
      //temp_fptr = temp_fptr + minBytes;   
      fs_setmaskbit(j);   
      i++;
    }
    
    if (i >= blocks_count)
    {
      break;
    }
  }

  if (data_in_bytes == 0)
  {
    printf("\n fs_write : all data written"); 
    oft[fd].fileptr = nbytes - data_in_bytes;  
  }
  else
  {
    printf("\n all data not written");
    oft[fd].fileptr = nbytes - data_in_bytes;
    return SYSERR;
  }

  in_temp.size = blocks_count;
  if((status = fs_put_inode_by_num(0, in_temp.id, &in_temp))==SYSERR){ 
    printf("\n fs_write: Could not write inode.");
    return SYSERR;
  }
  printf("\n indoe put back %d  size %d", in_temp.id, in_temp.size );
  printf("\n fileptr, data_in_bytes, minBytes");
  
  //return oft[fd].fileptr;
  return nbytes;
}

int fs_link(char *src_filename, char* dst_filename) 
{
  int inode_number = 0;
  int i =0;
  for( i=0; i<fsd.root_dir.numentries; i++)
  {
    if(strcmp(fsd.root_dir.entry[i].name, src_filename)==0)
    {
      printf("\n fs_link : File to be linked found");
      inode_number = i;
      break;
    }
  }

  if (i >= fsd.root_dir.numentries) 
  {
     printf("\n fs_link : File not present");
     return SYSERR;    
  }
  struct inode in;
  int status;
  if((status = fs_get_inode_by_num(0, inode_number, &in)) == SYSERR){
    printf("\n fs_link : Failed to retrieve inode from FSD");;
    return SYSERR;
  }

  in.nlink++;

  if((status = fs_put_inode_by_num(0, inode_number, &in))==SYSERR){
    printf("\n fs_link : fs_put_inode_by_num() failed");
    return SYSERR;
  }

  fsd.root_dir.entry[fsd.root_dir.numentries].inode_num = inode_number;
  strcpy(fsd.root_dir.entry[fsd.root_dir.numentries].name, dst_filename);
  fsd.root_dir.numentries++;
  return OK;
}

int fs_unlink(char *filename) 
{
  int inode_number = 0;
  int i =0;
  for( i=0; i<fsd.root_dir.numentries; i++)
  {
    if(strcmp(fsd.root_dir.entry[i].name, filename)==0)
    {
      printf("\n fs_create : File to be unlinked found");
      inode_number = i;
      break;
    }
  }

  if (i >= fsd.root_dir.numentries) 
  {
     printf("File not present");
     return SYSERR;    
  }

  struct inode in;
  int status;
  if((status = fs_get_inode_by_num(0, inode_number, &in)) == SYSERR){
    printf("\n fs_link : Failed to retrieve inode from FSD");;
    return SYSERR;
  }

  /*if ( in.nlink == 0)
  {
  
    while (in.size>0)
    {
      if(fs_clearmaskbit(in.size-1) != OK)
      {
        printf("\n fs_write :Error in clearing block %d",in.size-1);
        return SYSERR;
      }
      in.size--;
    }
    printf("\nfs_unlink : file delete successfully");
  }*/
  

  if ( in.nlink > 1)
  {
    in.nlink--;
  }
  if((status = fs_put_inode_by_num(0, inode_number, &in))==SYSERR){
    printf("\n fs_link : fs_put_inode_by_num() failed");
    return SYSERR;
  }

  fsd.root_dir.entry[fsd.root_dir.numentries].inode_num = 0;
  filename = NULL;
  strcpy(fsd.root_dir.entry[fsd.root_dir.numentries].name, filename);
  fsd.root_dir.numentries--;
  
  return OK;
}
#endif /* FS */
  