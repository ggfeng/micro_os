#include <stdio.h>
#include <hapi/nv.h>
#include <yodalite_autoconf.h>
#include <lib/property/properties.h>

#ifdef CONFIG_MEM_EXT_ENABLE
#include <lib/mem_ext/mem_ext.h>
#define yodalite_malloc malloc_ext
#define yodalite_free free_ext
#else
#define yodalite_malloc malloc
#define yodalite_free free
#endif

#define NV_SN_KEY       "ro.boot.serialno"
#define NV_SEED_KEY     "ro.boot.seed"
#define NV_DEVICETYPEID "ro.boot.devicetypeid"

int nv_prop_init(void)
{
#if CONFIG_PROPERTY_ENABLE == 1
  char * pBuf=NULL;
  if((pBuf= yodalite_malloc(MSG_LENGTH)) == NULL){
     printf("malloc %d  bytes fail\n",MSG_LENGTH);
     return -1;
  }
 
  if(read_seed(pBuf) == 0)
      property_set(NV_SEED_KEY,pBuf);
   else
      property_set(NV_SEED_KEY,"");

  if(read_sn(pBuf) ==0)
      property_set(NV_SN_KEY,pBuf);
   else
      property_set(NV_SN_KEY,"");

  if(read_devicetypeid(pBuf)==0)
      property_set(NV_DEVICETYPEID,pBuf);
   else
      property_set(NV_DEVICETYPEID,"");

  yodalite_free(pBuf);
#endif

  return 0;
}

int nv_prop_deinit(void)
{
   return 0;
}
