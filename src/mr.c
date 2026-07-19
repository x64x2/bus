#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "mr.h"
#include "spi.h"

inline unsigned char SHORT_READ_CMD(unsigned char addr) {
  assert(addr < 64);
  return (addr << 1);
}

inline unsigned char SHORT_WRITE_CMD(unsigned char addr) {
  assert(addr < 64);
  return (addr << 1) | 0x01;
}


int mrf24j40_short_read(BP * bp, unsigned char addr, unsigned char * value) {
  unsigned char data[]= { SHORT_READ_CMD(addr), 0xFF };
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  if (bp_bin_spi_bulk(bp, data, 2) < 0)
    return -1;
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  if (value != NULL)
    *value= data[1];
  return 0;
}

int mrf24j40_short_write(BP * bp, unsigned char addr, unsigned char value) {
  unsigned char data[]= { SHORT_WRITE_CMD(addr), value };
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  if (bp_bin_spi_bulk(bp, data, 2) < 0)
    return -1;
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  return 0;
}

int mrf24j40_long_read(BP * bp, unsigned short addr, unsigned char * value) {
  assert(addr < 0x3FF);
  unsigned char data[]= { 0x80 | (addr >> 3), (addr << 5) & 0xF0, 0xFF };
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  if (bp_bin_spi_bulk(bp, data, 3) < 0)
    return -1;
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;  
  if (value != NULL)
    *value= data[2];
  return 0;
}

int mrf24j40_long_write(BP * bp, unsigned short addr, unsigned char value) {
  assert(addr < 0x3FF);
  unsigned char data[]= { 0x80 | (addr >> 3), ((addr << 5) & 0xF0) | 0x10, value };
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;
  if (bp_bin_spi_bulk(bp, data, 3) < 0)
    return -1;
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;  
  return 0;
}

#define SHORT_WRITE(ADDRESS, VALUE) \
  if (mrf24j40_short_write(bp, MRF24J40_SREG_##ADDRESS, VALUE) < 0) \
    return -1;
#define SHORT_READ(ADDRESS, VALUE_PTR) \
  if (mrf24j40_short_read(bp, MRF24J40_SREG_##ADDRESS, VALUE_PTR) < 0) \
    return -1;
#define LONG_WRITE(ADDRESS, VALUE) \
  if (mrf24j40_long_write(bp, MRF24J40_LREG_##ADDRESS, VALUE) < 0) \
    return -1;
#define LONG_READ(ADDRESS, VALUE_PTR) \
  if (mrf24j40_long_read(bp, MRF24J40_LREG_##ADDRESS, VALUE_PTR) < 0) \
    return -1;

int mrf24j40_init(BP * bp, unsigned short panid, unsigned short sadr,
		  unsigned char * ladr, unsigned char channel,
		  int coordinator) {
  unsigned char value;
  
  SHORT_WRITE(SOFTRST, 0x07); 
  SHORT_WRITE(PACON2, 0x98);
  SHORT_WRITE(TXSTBL, 0x95); 
  LONG_WRITE(RFCON1, 0X01); 
  LONG_WRITE(RFCON2, 0x80); 
  LONG_WRITE(RFCON6, 0x90); 
  LONG_WRITE(RFCON7, 0x80); 
  LONG_WRITE(RFCON8, 0x10); 
  LONG_WRITE(SLPCON1, 0x21); 

  if (coordinator) {
    SHORT_READ(RXMCR, &value);
    SHORT_WRITE(RXMCR, value | 0x08); 
    SHORT_WRITE(ORDER, 0xFF); /* BO <7:4>= 0xF, SO <3:0> = 0xF */
  } else {
    SHORT_READ(RXMCR, &value);
    SHORT_WRITE(RXMCR, value & 0xF7);
  }
  SHORT_READ(TXMCR, &value);
  SHORT_WRITE(TXMCR, value & 0xDF); 
  
  SHORT_WRITE(BBREG2, 0x80); 
  SHORT_WRITE(CCAEDTH, 0x60); 
  SHORT_WRITE(BBREG6, 0x40); 

  SHORT_READ(INTCON, &value);
  SHORT_WRITE(INTCON, value | 0x08); 
  
  LONG_READ(SLPCON0, &value);
  LONG_WRITE(SLPCON0, value | 0x02); 

  assert((channel >= 11) && (channel <= 26)); 
  LONG_WRITE(RFCON0, ((channel-11) << 4) | 0x02);
  usleep(192);

  /*flush the RX FIFO
     - bit 0=1 flush RX FIFO (cleared by hardware) */
  SHORT_WRITE(RXFLUSH, 0x01);

  SHORT_WRITE(PANIDL, panid & 0xFF);
  SHORT_WRITE(PANIDH, panid >> 8);

  SHORT_WRITE(SADRL, sadr & 0xFF);
  SHORT_WRITE(SADRH, sadr >> 8);

  char i;
  for (i= 0; i < 8; i++)
    SHORT_WRITE(EADR0+i, *(ladr+i));

  return 0;
}

int mrf24j40_promiscuous(BP * bp, int error) {
  unsigned char value;

  SHORT_READ(RXMCR, &value);
  value|= 0x01;
  if (error)
    value|= 0x02;
  SHORT_WRITE(RXMCR, value); 
  return 0;
}

#define TXN_FIFO_WRITE(OFFS, VALUE) \
  if (mrf24j40_long_write(bp, MRF24J40_MEM_OFFSET_TXN_FIFO+OFFS, VALUE) < 0) \
    return -1;
#define RX_FIFO_READ(OFFS, VALUE_PTR) \
  if (mrf24j40_long_read(bp, MRF24J40_MEM_OFFSET_RX_FIFO+OFFS, VALUE_PTR) < 0) \
    return -1;

int mrf24j40_tx(BP * bp, unsigned short panid,
		unsigned short sadr, unsigned short dadr,
		unsigned char seqnum, int ack_request,
		unsigned char * payload, unsigned char payload_len) {
  unsigned char value;

  TXN_FIFO_WRITE(0, 0);
  TXN_FIFO_WRITE(1, 13);
  TXN_FIFO_WRITE(2, 0x01 | (ack_request?0x20:0));

  unsigned char da_mode= 0x02;
  unsigned char sa_mode= 0x02; 
  TXN_FIFO_WRITE(3, ((sa_mode << 6) | (da_mode << 2)));
  TXN_FIFO_WRITE(4, seqnum);
  TXN_FIFO_WRITE(5, panid & 0xFF);
  TXN_FIFO_WRITE(6, panid >> 8);
  TXN_FIFO_WRITE(7, dadr & 0xFF);
  TXN_FIFO_WRITE(8, dadr >> 8);
  TXN_FIFO_WRITE(9, panid & 0xFF);
  TXN_FIFO_WRITE(10, panid >> 8);
  TXN_FIFO_WRITE(11, sadr & 0xFF);
  TXN_FIFO_WRITE(12, sadr >> 8);
  TXN_FIFO_WRITE(13, 0x5A);
  TXN_FIFO_WRITE(14, 0xA5);
  
  unsigned char txncon= TXNCON_TXNTRIG;
  if (ack_request)
    txncon|= TXNCON_TXNACKREQ;
  SHORT_WRITE(TXNCON, txncon); 
  
  int ncount= 0;
  do {
    SHORT_READ(INTSTAT, &value);
    if (value & INTSTAT_TXNIF) {
      printf("TXIF set :-)\n");
      SHORT_READ(TXSTAT, &value);
      if (value & TXSTAT_CCAFAIL) {
	printf("CCA failed, channel busy :-(\n");
      } else {
	printf("CCA passed :-)\n");
      }
      printf("Retry times=%u\n", (value & TXSTAT_TXNRETRY) >> 6);
      if (!(value & TXSTAT_TXNSTAT)) {
	printf("ncount=%d\n", ncount);
	return 1;
      } else {
	printf("ncount=%d\n", ncount);
	return 0;
      }
    }
    ncount++;
  } while (ncount < 1000);
  printf("ncount=%d\n", ncount);
  return -1;
}

int mrf24j40_rx(BP * bp, mac_frame * frame, unsigned char * rssi,
		unsigned char * lqi) {
  unsigned char value;
  unsigned char rx_fifo[144]; 
  int i;

  SHORT_READ(INTSTAT, &value);
  if (!(value & 0x08)) 
    return 0;

  for (i= 1; i < 144; i++)
    RX_FIFO_READ(i, &rx_fifo[i]);
  RX_FIFO_READ(0, &rx_fifo[0]);

  unsigned char len= rx_fifo[0];
  assert(len <= 127);

  for (i= 0; i <= len; i++)
    printf(" %.2X", rx_fifo[i]);
  printf("\n");

  if (frame != NULL) {
    int index= 1;

    frame->type= rx_fifo[index] & 0x07;
    index++;

    frame->src_addr.mode= (rx_fifo[index] >> 6) & 0x03;
    frame->dst_addr.mode= (rx_fifo[index] >> 2) & 0x03;
    index++;
    frame->seqnum= rx_fifo[index++];

    switch (frame->dst_addr.mode) {
    case 0x02:
      frame->dst_addr.m2.panid= (rx_fifo[index+1] << 8) | rx_fifo[index];
      frame->dst_addr.m2.sadr= (rx_fifo[index+3] << 8) | rx_fifo[index+2];
      index+= 4;
      break;
    case 0x03:
      memcpy(frame->src_addr.m3.ladr, &rx_fifo[index], MAC_ADDR_M3_LEN);
      index+= 8;
      break;
    }

    switch (frame->src_addr.mode) {
    case 0x02:
      frame->src_addr.m2.panid= (rx_fifo[index+1] << 8) | rx_fifo[index];
      frame->src_addr.m2.sadr= (rx_fifo[index+3] << 8) | rx_fifo[index+2];
      index+= 4;
      break;
    case 0x03:
      memcpy(frame->src_addr.m3.ladr, &rx_fifo[index], MAC_ADDR_M3_LEN);
      index+= 8;
      break;
    }
    memcpy(frame->payload, &rx_fifo[index], len-2-(index-1));
    frame->crc= (rx_fifo[len] << 8) | rx_fifo[len-1];
    frame->len= len-2-(index-1);
  }
  if (rssi != NULL)
    *rssi= rx_fifo[len+1];
  if (lqi != NULL)
    *lqi= rx_fifo[len+2];
  return 1;
}

inline void byte2binstr(unsigned char b, char * buf, size_t len) {
  int i;
  assert(len >= 9);
  for (i= 0; i < 8; i++)
    buf[i]= ((b & (1 << (7-i))) != 0)?'1':'0';
  buf[8]= '\0';
}

int mrf24j40_xtalk_test(BP * bp, int num_meas) {
  const unsigned char short_reg_addr= MRF24J40_SREG_PANIDL;
  const unsigned short long_reg_addr= MRF24J40_MEM_OFFSET_SEC_BUF;
  int count= 0, old_count= 0;
  int total= 0;

  unsigned char w_value= 0;
  unsigned char r_value;
  
  while ((num_meas > 0) || (num_meas < 0)) {
    if (mrf24j40_long_write(bp, long_reg_addr, w_value) < 0)
      return -1;
    if (mrf24j40_long_read(bp, long_reg_addr, &r_value) < 0)
      return -1;
    if (r_value != w_value) {
      char r_buf[9], w_buf[9];
      byte2binstr(w_value, w_buf, sizeof(w_buf));
      byte2binstr(r_value, r_buf, sizeof(r_buf));
      printf("Mismatch: %X (%s) -?-> %X (%s)\n",
	     w_value, w_buf, r_value, r_buf);
      count++;
    }
    total++;
    if ((old_count != count) || (total % 100 == 0)) {
      printf("\rNumber of mismatches: %d/%d (%.1f%%)", count, total,
	     ((double) count)/total*100);
      fflush(stdout);
      old_count= count;
    }
    w_value++;
    if (num_meas > 0)
      num_meas--;
  }
  printf("\rNumber of mismatches: %d/%d (%.1f%%)\n", count, total,
	 ((double) count)/total*100);
  return count;
}
