#undef __BUSPIRATE_SPI_H__
#define __BUSPIRATE_SPI_H__

#include "bus.h"
#define BP_BIN_SPI_SPEED_30K  0
#define BP_BIN_SPI_SPEED_125K 1
#define BP_BIN_SPI_SPEED_250K 2
#define BP_BIN_SPI_SPEED_1M   3
#define BP_BIN_SPI_SPEED_2M   4
#define BP_BIN_SPI_SPEED_2_6M 5
#define BP_BIN_SPI_SPEED_4M   6
#define BP_BIN_SPI_SPEED_8M   7

#define BP_BIN_SPI_LV_3V3        0x08
#define BP_BIN_SPI_LV_HIZ        0x00

#define BP_BIN_SPI_CLK_IDLE_HIGH 0x04
#define BP_BIN_SPI_CLK_IDLE_LOW  0x00

#define BP_BIN_SPI_CLK_EDGE_HIGH 0x02
#define BP_BIN_SPI_CLK_EDGE_LOW  0x00

#define BP_BIN_SPI_SMP_END       0x01
#define BP_BIN_SPI_SMP_MIDDLE    0x00

#define BP_BIN_SPI_VERSION     0x01
#define BP_BIN_SPI_CS_LOW      0x02
#define BP_BIN_SPI_CS_HIGH     0x03
#define BP_BIN_SPI_BULK        0x10
#define BP_BIN_SPI_SET_PERIPH  0x40
#define BP_BIN_SPI_GET_PERIPH  0x50
#define BP_BIN_SPI_SET_SPEED   0x60
#define BP_BIN_SPI_GET_SPEED   0x70
#define BP_BIN_SPI_SET_CONFIG  0x80
#define BP_BIN_SPI_GET_CONFIG  0x90

#ifdef __cplusplus
extern "C" {
#endif

  int bp_bin_spi_version(BP * bp, unsigned char * version);
  int bp_bin_spi_get_speed(BP * bp, unsigned char * speed);
  int bp_bin_spi_set_speed(BP * bp, unsigned char speed);
  int bp_bin_spi_get_config(BP * bp, unsigned char * config);
  int bp_bin_spi_set_config(BP * bp, unsigned char config);
  int bp_bin_spi_get_periph(BP * bp, unsigned char * config);
  int bp_bin_spi_set_periph(BP * bp, unsigned char config);
  int bp_bin_spi_cs(BP * bp, int enabled);

  int bp_bin_spi_bulk(BP * bp, unsigned char * data, unsigned char nlen);

#ifdef __cplusplus
}
#endif
