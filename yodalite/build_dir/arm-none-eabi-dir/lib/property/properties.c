#include <yodalite_autoconf.h>
#include <lib/property/properties.h>

#include <stdlib.h>
#include <string.h>

#include <lib/list/list.h>
#include <lib/fatfs/ff.h>
#include <osal/pthread.h>


#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

#define fd_t     FIL
#define PROPERTY_SAVE_HEAD "persist"
#define PROPERTY_FILE "0:property.txt"


typedef struct 
{
   uint8_t key[PROPERTY_KEY_MAX];
   uint8_t value[PROPERTY_VALUE_MAX];
}property_t;

typedef struct
{
  struct list_head list;
  property_t property;
}property_list_t;

static pthread_mutex_t g_property_mutex;
static property_list_t g_property_lists;

static int property_open_file(char *file,fd_t*fd,uint8_t mode)
{
  FRESULT fr;

  fr = f_open(fd,file,mode);

  if (fr){
      printf("#Err:f_open() = %d\n",fr);
      return -1;
  }

  return 0;
}

static int property_close_file(fd_t*fd)
{
  FRESULT fr; 

  fr = f_close(fd);

  if (fr){
      printf("#Err:f_close() = %d\n",fr);
      return -1;
  }

  return 0;
}

static int property_read_file(fd_t *fd,uint8_t* buf,uint32_t size)
{

  FRESULT fr; 
  uint32_t bw;
  
  fr = f_read(fd,buf,size,&bw);

  if (fr ||  bw != size){
      printf("f_read() %d bytes fr= %d\n",bw,fr);
      return -1;
  }

  return 0;
}

static int property_write_file(fd_t *fd,uint8_t* buf,uint32_t size)
{
  FRESULT fr; 
  uint32_t bw;
  property_t * pos = (property_t*) buf;

//  printf("%s:%s\n",pos->key,pos->value);
  fr = f_write(fd,buf,size,&bw);

  if (fr ||  bw != size){
      printf("f_write() %d bytes fr= %d\n",bw,fr);
      return -1;
  }

  return 0;
}

static int property_seek_file(fd_t*fd,int off)
{
  FRESULT fr; 

  fr = f_lseek(fd,(FSIZE_t)off);

  if(fr){
     printf("f_lseek() fail\n");
     return -1;
  }
  return 0;
}

static int property_tell_file(fd_t *fd)
{
  return f_size(fd);
}

static int write_property(void)
{
  fd_t fd;
  property_list_t *pos;
  property_list_t*head = &g_property_lists;

  if(property_open_file(PROPERTY_FILE ,&fd,FA_CREATE_ALWAYS | FA_WRITE)){
     printf("error:property_open_file %s fail\n",PROPERTY_FILE);
     return -1;
  }

   list_for_each_entry(pos, &(head->list),list){
      if(strstr(pos->property.key,PROPERTY_SAVE_HEAD)){
        if(property_write_file(&fd,(uint8_t *)&pos->property,sizeof(property_t))){
          printf("error:property_write_file() fail\n");
          property_close_file(&fd);
          return -1;
      }
    }
  }

  return property_close_file(&fd);
}

static int property_list_init(void)
{
  fd_t fd;
  property_t property;
  property_list_t*head = &g_property_lists;

  if(property_open_file(PROPERTY_FILE ,&fd,FA_OPEN_ALWAYS| FA_READ)){
     printf("error:property_open_file %s fail\n",PROPERTY_FILE);
     return -1;
  }

//  printf("%s file size:%d,eof= %d\n",PROPERTY_FILE,property_tell_file(&fd),f_eof(&fd));
  while(!f_eof(&fd))
  {
     property_list_t *pos;

     memset(&property,0,sizeof(property_t));

     if(property_read_file(&fd,(uint8_t *)&property,sizeof(property_t))){
       printf("error:property_read_file() fail\n");
       property_close_file(&fd);
       return -1;
     }

     if((pos = (property_list_t*)yodalite_malloc(sizeof(property_list_t))) == NULL){
     printf("error:yodalite_malloc %d fail\n",sizeof(property_list_t));
     property_close_file(&fd);
     return -1;
   }

    printf("%s:%s\n",property.key,property.value);
    strcpy(pos->property.key,property.key);
    strcpy(pos->property.value,property.value);

    list_add_tail(&pos->list,&head->list);
  }

  return property_close_file(&fd);
}

int property_init(void)
{
    property_list_t*head = &g_property_lists;

    pthread_mutex_init(&g_property_mutex,NULL);

    pthread_mutex_lock(&g_property_mutex);
    INIT_LIST_HEAD(&head->list);
    property_list_init();  
    pthread_mutex_unlock(&g_property_mutex);

    return 0;
}

int property_deinit(void)
{
    property_list_t *pos,*n;
    property_list_t *head = &g_property_lists;

    pthread_mutex_lock(&g_property_mutex);
    list_for_each_entry_safe(pos, n,&head->list,list){
        list_del(&pos->list);
        free(pos);
    } 
    pthread_mutex_unlock(&g_property_mutex);
    return 0;
}

int property_print_list(void)
{
    property_list_t *pos;
    property_list_t *head = &g_property_lists;

    pthread_mutex_lock(&g_property_mutex);
    list_for_each_entry(pos, &(head->list),list){
      printf("%s:%s\n",pos->property.key,pos->property.value);
    }
    pthread_mutex_unlock(&g_property_mutex);

   return 0;
}


int property_get(const char *key,char *value,const char *default_value)
{
    property_list_t *pos;
    property_list_t *head = &g_property_lists;

    if(key == NULL || default_value == NULL){
       printf("error:key:%p,default_value:%p\n",key,default_value);
       return -1;
     }

    if(strlen(key) > PROPERTY_KEY_MAX-1 || 
       strlen(default_value) > PROPERTY_VALUE_MAX-1){
        printf("error:key(%d) > %d,default_value(%d) > %d\n",
         strlen(key),PROPERTY_KEY_MAX-1 ,strlen(default_value)-1,PROPERTY_VALUE_MAX-1);
        return -1;
     }
      
    pthread_mutex_lock(&g_property_mutex);
      list_for_each_entry(pos, &(head->list), list){
       if(strcmp (pos->property.key,key)==0){
         strcpy(value,pos->property.value);
         pthread_mutex_unlock(&g_property_mutex);
         return 1;
       }
    }
    pthread_mutex_unlock(&g_property_mutex);

    strcpy(value,default_value);

    return 0;
}

int property_set(const char *key,const char *value)
{
  int iret = 0;
  property_list_t *pos;
  property_list_t *head = &g_property_lists;


  if(key == NULL || value == NULL){
     printf("error:key:%s,value:%s\n",key,value);
     return -1;
   }

  if(strlen(key) > PROPERTY_KEY_MAX-1){
     printf("error:key(%d) > %d\n",strlen(key),PROPERTY_KEY_MAX-1);
     return -1;
   }

  if(strlen(value) > PROPERTY_VALUE_MAX -1){
     printf("error:value(%d) > %d\n",strlen(value),PROPERTY_VALUE_MAX-1);
     return -1;
   }

   pthread_mutex_lock(&g_property_mutex);

   list_for_each_entry(pos, &(head->list), list){
       if(strcmp (pos->property.key,key)==0){
         strcpy(pos->property.value,value);
         goto save;
       }
   }

  if((pos = (property_list_t*)yodalite_malloc(sizeof(property_list_t))) == NULL){
     printf("error:yodalite_malloc %d fail\n",sizeof(property_list_t));
    pthread_mutex_unlock(&g_property_mutex);
     return -1;
   }

  strcpy(pos->property.key,key);
  strcpy(pos->property.value,value);

  list_add_tail(&pos->list,&head->list);

save:
  if(strstr(pos->property.key,PROPERTY_SAVE_HEAD)){
//  printf("save %s:%s,%d bytes\n",pos->property.key,pos->property.value,sizeof(property_t));
    iret = write_property();
   }

   pthread_mutex_unlock(&g_property_mutex);
  return iret; 
}

int property_remove(const char *key)
{
    int iret = 0;
    property_list_t *pos,*n;
    property_list_t *head = &g_property_lists;

    if(key == NULL){
       printf("error:key:%p\n",key);
       return -1;
     }

    if(strlen(key) > PROPERTY_KEY_MAX-1){
        printf("error:key(%d) > %d\n",strlen(key),PROPERTY_KEY_MAX-1);
        return -1;
     }
      
    pthread_mutex_lock(&g_property_mutex);

    list_for_each_entry_safe(pos, n,&head->list,list){
       if(strcmp (pos->property.key,key)==0){
        list_del(&pos->list);
        yodalite_free(pos);
      }
    }
 
  if(strstr(key,PROPERTY_SAVE_HEAD)){
    iret = write_property();
   }
   pthread_mutex_unlock(&g_property_mutex);

   return iret;
}
