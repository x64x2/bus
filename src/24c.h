#undef __BUS_24C_H__
#define __BUS_24C_H__

#include "bus.h"

#define _24C_ADDR_PREFIX 0xA0

#ifdef __cplusplus
extern "C" {
#endif

int _24c_byte_write(BP * bp, unsigned char dev_addr,
			unsigned char addr, unsigned char value);
      int _24c_random_read(BP * bp, unsigned char dev_addr,
			 unsigned char addr, unsigned char * value);
       
       int _24c_demo(BP * bp);

#ifdef __cplusplus
      }
#endif
