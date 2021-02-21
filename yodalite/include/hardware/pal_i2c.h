#ifndef _PAL_I2C_H_
#define _PAL_I2C_H_

struct i2c_resource {
    unsigned i2c_id;
    unsigned i2c_client;
    unsigned i2c_clk;
};

struct i2c_lapi {
    int (*yodalite_i2c_bus_init)(struct i2c_resource *pi2c_res);
    int (*yodalite_i2c_read)(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len);
    int (*yodalite_i2c_write)(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len);
    int (*yodalite_i2c_block_read)(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len);
    int (*yodalite_i2c_block_write)(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len);

};

extern int pal_i2c_init(struct i2c_lapi *platform_i2c_lapi);
extern int pal_i2c_bus_init(struct i2c_resource *pi2c_res);
extern int pal_i2c_read(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len);
extern int pal_i2c_write(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len);
extern int pal_i2c_block_read(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len);
extern int pal_i2c_block_write(unsigned i2c_id, unsigned char client, unsigned char addr,unsigned char mode,unsigned char *pdata,unsigned char len);

#endif  /*_PAL_I2C_H_*/

