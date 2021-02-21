#ifndef _PAL_SPI_H_
#define _PAL_SPI_H_

struct spi_resource {
    unsigned spi_id;
    unsigned spi_clk;
};

struct spi_lapi {
    int (*yodalite_spi_bus_init)(struct spi_resource *pspi_res);
    int (*yodalite_spi_read)(unsigned spi_id, unsigned char *buf, unsigned len);
    int (*yodalite_spi_write)(unsigned spi_id, unsigned char *buf, unsigned len);
};

extern int pal_spi_init(struct spi_lapi *platform_spi_lapi);
extern int pal_spi_bus_init(struct spi_resource *pspi_res);
extern int pal_spi_read(unsigned spi_id, unsigned char *buf, unsigned len);
extern int pal_spi_write(unsigned spi_id, unsigned char *buf, unsigned len);

#endif  /*_PAL_SPI_H_*/

