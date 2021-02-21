#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <lib/fatfs/ff.h>
#include <yodalite_autoconf.h>
#include <lib/shell/shell.h>

#define FATFS_BUF_SIZE (4096)
#define FATFS_TEST_SIZE (256)

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

static FATFS fs;  
static FIL fd;   
static    BYTE src[FATFS_TEST_SIZE]; 
static    BYTE dst[FATFS_TEST_SIZE]; 


#define DEFAULT_FATFS_LABEL "fatfs"

static int fatfs_format(int argc,char *const argv[])
{
   int iret=0;
   FRESULT fr; 
   BYTE *buf;
   char drv[3] = {(char)('0' + 0), ':', 0};

   printf("f_mkfs...\n");

   buf = yodalite_malloc(FATFS_BUF_SIZE);

   if(buf== NULL){
     printf("malloc %d fail\n",FATFS_BUF_SIZE);
     return -1;
   }

   fr = f_mkfs(drv,FM_ANY,0,buf,FATFS_BUF_SIZE);

   yodalite_free(buf);

   if(fr){
         printf("#Err:f_mkfs()= %d\n",fr);
         return (int)fr;
      }
   
    printf("f_format() = %d\n",fr);

  return iret;

}

void dump_data(BYTE *buf,int bytes)
{
   int i;

   for(i=0;i<bytes;i++)
   {

     if(i %16 == 0)printf("0x%04x:",i);

      printf("%02x ",buf[i]);

      if((i+1) %16 == 0)
       printf("\n");
   }
}


static int fatfs_access(int argc,char * const argv[])
{

    int i;

    FRESULT fr;          /* FatFs function common result code */
    UINT bw;         /* File read/write count */
    static BYTE cnts=0;


  //  printf("fatfs access sector:0x%x\n",sizeof(fs.win));
  //  memset(&fs,0,sizeof(FATFS));

    for(i=0;i<FATFS_TEST_SIZE;i++)
    {
       src[i] = (i + cnts) & 0xff;
       dst[i] = 0;
    }

    cnts ++;


    fr = f_open(&fd, "0:file.bin", FA_WRITE | FA_CREATE_ALWAYS);

    if(fr) 
    {
      printf("#Err:f_open() = %d\n",fr);
      return (int)fr;
    }

    fr = f_write(&fd,src,FATFS_TEST_SIZE, &bw);            /* Write it to the destination file */

    if (fr ||  bw != FATFS_TEST_SIZE) 
    {
      printf("f_write() %d bytes fr= %d\n",bw,fr);
      return (int)fr;
    }

    f_close(&fd);

    fr = f_open(&fd, "0:file.bin", FA_READ);

    if (fr) 
    {
      printf("#Err:f_open() = %d\n",fr);
      return (int)fr;
    }

    fr = f_read(&fd,dst,FATFS_TEST_SIZE,&bw); 

    if (fr ||  bw != FATFS_TEST_SIZE) 
    {
      printf("f_read() %d bytes fr= %d\n",bw,fr);
      return (int)fr;
    }

    if(memcmp(src,dst,FATFS_TEST_SIZE) == 0)
    {
      printf("fatfs read write succeed!\n");
    }
    else
    {
      printf("fatfs read write unsucceed!\n");
    }
    
    dump_data(dst,FATFS_TEST_SIZE);

    f_close(&fd);

    return 0;
}


static int fatfs_read(int argc,char * const argv[])
{
    int i;

    FRESULT fr;          /* FatFs function common result code */
    UINT bw;         /* File read/write count */


 //   memset(&fs,0,sizeof(FATFS));

    for(i=0;i<FATFS_TEST_SIZE;i++)
    {
       dst[i] = 0;
    }


    fr = f_open(&fd, "0:file.bin", FA_READ);

    if (fr) 
    {
      printf("#Err:f_open() = %d\n",fr);
      return (int)fr;
    }

    fr = f_read(&fd,dst,FATFS_TEST_SIZE,&bw); 

    if (fr ||  bw != FATFS_TEST_SIZE) 
    {
      printf("f_read() %d bytes fr= %d\n",bw,fr);
      return (int)fr;
    }

    dump_data(dst,FATFS_TEST_SIZE);

    f_close(&fd);

    return 0;
}

#define format_max_args   (1)
#define format_help  "fatfs_format"

int cmd_fatfs_format(void)
{
    YODALITE_REG_CMD(fatfs_format,format_max_args,fatfs_format,format_help);

    return 0;
}


#define a_max_args   (1)
#define access_help  "fatfs_access"

int cmd_fatfs_access(void)
{

    YODALITE_REG_CMD(fatfs_access,a_max_args,fatfs_access,access_help);

    return 0;
}

#define r_max_args   (1)
#define read_help  "fatfs_read"

int cmd_fatfs_read(void)
{

    YODALITE_REG_CMD(fatfs_read,r_max_args,fatfs_read,read_help);

    return 0;
}
