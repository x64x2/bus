#undef __MR_H__
#define __MR_H__

#include "bus.h"
#include "802.h"

#define MRF24J40_SREG_RXMCR     0x00
#define MRF24J40_SREG_PANIDL    0x01
#define MRF24J40_SREG_PANIDH    0x02
#define MRF24J40_SREG_SADRL     0x03
#define MRF24J40_SREG_SADRH     0x04
#define MRF24J40_SREG_EADR0     0x05
#define MRF24J40_SREG_EADR1     0x06
#define MRF24J40_SREG_EADR2     0x07
#define MRF24J40_SREG_EADR3     0x08
#define MRF24J40_SREG_EADR4     0x09
#define MRF24J40_SREG_EADR5     0x0A
#define MRF24J40_SREG_EADR6     0x0B
#define MRF24J40_SREG_EADR7     0x0C
#define MRF24J40_SREG_RXFLUSH   0x0D

#define MRF24J40_SREG_ORDER     0x10
#define MRF24J40_SREG_TXMCR     0x11
#define MRF24J40_SREG_ACKTMOUT  0x12
#define MRF24J40_SREG_ESLOTG1   0x13
#define MRF24J40_SREG_SYMTICKL  0x14
#define MRF24J40_SREG_SYMTICKH  0x15
#define MRF24J40_SREG_PACON0    0x16 
#define MRF24J40_SREG_PACON1    0x17 
#define MRF24J40_SREG_PACON2    0x18 

#define MRF24J40_SREG_TXBCON0   0x1A
#define MRF24J40_SREG_TXNCON    0x1B
#define MRF24J40_SREG_TXG1CON   0x1C
#define MRF24J40_SREG_TXG2CON   0x1D
#define MRF24J40_SREG_ESLOTG23  0x1E
#define MRF24J40_SREG_ESLOTG45  0x1F
#define MRF24J40_SREG_ESLOTG67  0x20
#define MRF24J40_SREG_TXPEND    0x21
#define MRF24J40_SREG_WAKECON   0x22
#define MRF24J40_SREG_FRMOFFSET 0x23
#define MRF24J40_SREG_TXSTAT    0x24
#define MRF24J40_SREG_TXBCON1   0x25
#define MRF24J40_SREG_GATECLK   0x26
#define MRF24J40_SREG_TXTIME    0x27
#define MRF24J40_SREG_HSYMTMRL  0x28
#define MRF24J40_SREG_HSYMTMRH  0x29
#define MRF24J40_SREG_SOFTRST   0x2A 
#define MRF24J40_SREG_SECCON0   0x2C
#define MRF24J40_SREG_SECCON1   0x2D
#define MRF24J40_SREG_TXSTBL    0x2E 
#define MRF24J40_SREG_RXSR      0x30

#define MRF24J40_SREG_INTSTAT   0x31 
#define MRF24J40_SREG_INTCON    0x32
#define MRF24J40_SREG_GPIO      0x33
#define MRF24J40_SREG_TRISGPIO  0x34
#define MRF24J40_SREG_SLPACK    0x35
#define MRF24J40_SREG_RFCTL     0x36 /
#define MRF24J40_SREG_SECCR2    0x37
#define MRF24J40_SREG_BBREG0    0x38
#define MRF24J40_SREG_BBREG1    0x39
#define MRF24J40_SREG_BBREG2    0x3A
#define MRF24J40_SREG_BBREG3    0x3B
#define MRF24J40_SREG_BBREG4    0x3C

#define MRF24J40_SREG_BBREG6    0x3E
#define MRF24J40_SREG_CCAEDTH   0x3F

#define MRF24J40_LREG_RFCON0    0x200
#define MRF24J40_LREG_RFCON1    0x201
#define MRF24J40_LREG_RFCON2    0x202
#define MRF24J40_LREG_RFCON3    0x20

#define MRF24J40_LREG_RFCON5    0x205
#define MRF24J40_LREG_RFCON6    0x206
#define MRF24J40_LREG_RFCON7    0x207
#define MRF24J40_LREG_RFCON8    0x208
#define MRF24J40_LREG_SLPCAL0   0x209
#define MRF24J40_LREG_SLPCAL1   0x20A
#define MRF24J40_LREG_SLPCAL2   0x20B

#define MRF24J40_LREG_RSSI      0x210
#define MRF24J40_LREG_SLPCON0   0x211

#define MRF24J40_LREG_SLPCON1   0x220

#define MRF24J40_LREG_WAKETIMEL 0x222
#define MRF24J40_LREG_WAKETIMEH 0x223
#define MRF24J40_LREG_REMCNTL   0x224
#define MRF24J40_LREG_REMCNTH   0x225
#define MRF24J40_LREG_MAINCNT0  0x226
#define MRF24J40_LREG_MAINCNT1  0x227
#define MRF24J40_LREG_MAINCNT2  0x228
#define MRF24J40_LREG_MAINCNT3  0x229

#define MRF24J40_LREG_TESTMODE  0x22F
#define MRF24J40_LREG_ASSOEADR0 0x230
#define MRF24J40_LREG_ASSOEADR1 0x231
#define MRF24J40_LREG_ASSOEADR2 0x232
#define MRF24J40_LREG_ASSOEADR3 0x233
#define MRF24J40_LREG_ASSOEADR4 0x234
#define MRF24J40_LREG_ASSOEADR5 0x235
#define MRF24J40_LREG_ASSOEADR6 0x236
#define MRF24J40_LREG_ASSOEADR7 0x237
#define MRF24J40_LREG_ASS0SADR0 0x238
#define MRF24J40_LREG_ASSOSADR1 0x239
/* 0x23A-0x23B reserved */
/* 0x23C-0x23E unimplemented */
#define MRF24J40_LREG_UPNONCE0  0x240
#define MRF24J40_LREG_UPNONCE1  0x241
#define MRF24J40_LREG_UPNONCE2  0x242
#define MRF24J40_LREG_UPNONCE3  0x243
#define MRF24J40_LREG_UPNONCE4  0x244
#define MRF24J40_LREG_UPNONCE5  0x245
#define MRF24J40_LREG_UPNONCE6  0x246
#define MRF24J40_LREG_UPNONCE7  0x247
#define MRF24J40_LREG_UPNONCE8  0x248
#define MRF24J40_LREG_UPNONCE9  0x249
#define MRF24J40_LREG_UPNONCE10 0x24A
#define MRF24J40_LREG_UPNONCE11 0x24B
#define MRF24J40_LREG_UPNONCE12 0x24C

#define TXSTAT_TXNSTAT   0x01
#define TXSTAT_CCAFAIL   0x20
#define TXSTAT_TXNRETRY  0xC0

#define INTSTAT_TXNIF    0x01
#define INTSTAT_RXIF     0x08

#define TXNCON_TXNTRIG   0x01
#define TXNCON_TXNACKREQ 0x04


#define MRF24J40_MEM_OFFSET_TXN_FIFO  0x000
#define MRF24J40_MEM_OFFSET_TXB_FIFO  0x080
#define MRF24J40_MEM_OFFSET_GTS1_FIFO 0x100
#define MRF24J40_MEM_OFFSET_GTS2_FIFO 0x180
#define MRF24J40_MEM_OFFSET_CTRL_REGS 0x200
#define MRF24J40_MEM_OFFSET_SEC_BUF   0x280
#define MRF24J40_MEM_OFFSET_RX_FIFO   0x300

#ifdef __cplusplus
extern "C" {
#endif

  int mrf24j40_short_read(BP * bp, unsigned char addr, unsigned char * value);
  int mrf24j40_short_write(BP * bp, unsigned char addr, unsigned char value);

  int mrf24j40_long_read(BP * bp, unsigned short addr, unsigned char * value);
  int mrf24j40_long_write(BP * bp, unsigned short addr, unsigned char value);

  int mrf24j40_init(BP * bp, unsigned short panid, unsigned short sadr,
		    unsigned char * ladr, unsigned char channel,
		    int coordinator);
  int mrf24j40_promiscuous(BP * bp, int error);
  int mrf24j40_tx(BP * bp, unsigned short panid,
		  unsigned short sadr, unsigned short dadr,
		  unsigned char seqnum, int ack_request,
		  unsigned char * payload, unsigned char payload_len);
  int mrf24j40_rx(BP * bp, mac_frame * frame, unsigned char * rssi,
		  unsigned char * lqi);

  int mrf24j40_xtalk_test(BP * bp, int num_meas);

#ifdef __cplusplus
}
#endif
