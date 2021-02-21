#ifndef _NV_H_
#define _NV_H_

#define MSG_LENGTH (128)

extern int read_seed(char *buf);
extern int write_seed(char *buf);
extern int read_sn(char *buf);
extern int write_sn(char *buf);
extern int read_devicetypeid(char *buf);
extern int write_devicetypeid(char *buf);
extern int nv_prop_init(void);
extern int nv_prop_deinit(void);
#endif
