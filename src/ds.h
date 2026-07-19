#undef __BUS_DS_H__
#define __BUS_DS_H__

#include "bus.h"

#define DS1624_ADDR_PREFIX       0x90
#define DS1624_CMD_STOP_CONVERT  0x22
#define DS1624_CMD_READ_TEMP     0xAA
#define DS1624_CMD_START_CONVERT 0xEE
#define DS1631_ADDR_PREFIX       0x90
#define DS1631_CMD_STOP_CONVERT  0x22
#define DS1631_CMD_READ_TEMP     0xAA
#define DS1631_CMD_START_CONVERT 0x51

#ifdef __cplusplus
extern "C" {
#endif

  int ds1624_demo(BP * bp);

#ifdef __cplusplus
}
#endif
