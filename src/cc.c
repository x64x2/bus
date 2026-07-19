#include <assert.h>
#include <stdio.h>
#include "cc.h"
#include "spi.h"

#define FXOSC 26000000

int cc2500_read_register(BP * bp, unsigned char reg, unsigned char * value)
{
  assert(value != NULL);
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;

  unsigned char data[]= { reg, 0xFF };
  if (bp_bin_spi_bulk(bp, data, 2) < 0)
    return -1;

  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;

  *value= data[1];
  return 0;
}

int cc2500_get_version(BP * bp, unsigned char * version,
		       unsigned char * partnum)
{
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;

  unsigned char data[]= { CC2500_REG_VERSION, 0xFF,
			  CC2500_REG_PARTNUM, 0xFF };
  if (bp_bin_spi_bulk(bp, data, 4) < 0)
    return -1;
  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;

  if (version != NULL)
    *version= data[1];
  if (partnum != NULL)
    *partnum= data[3];
  return 0;
}

int cc2500_get_channel(BP * bp, unsigned char * channel)
{
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;

  unsigned char data[]= { 0x80 + CC2500_REG_CHANNR, 0xFF };
  if (bp_bin_spi_bulk(bp, data, 2) < 0)
    return -1;

  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;
  return 0;
}

int cc2500_get_freq(BP * bp, unsigned long * freq)
{
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;

  unsigned char data[]= { 0x80 + CC2500_REG_FREQ2, 0xFF,
			  0x80 + CC2500_REG_FREQ1, 0xFF,
			  0x80 + CC2500_REG_FREQ0, 0xFF };
  if (bp_bin_spi_bulk(bp, data, 6) < 0)
    return -1;

  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;

  if (freq != NULL) {
    unsigned long the_freq= (data[1] << 16) + (data[3] << 8) + data[5];
    the_freq*= FXOSC;
    the_freq/= (1 << 16);
    *freq= the_freq;
  }
  return 0;
}

int cc2500_get_marcstate(BP * bp, unsigned char * marcstate)
{
  unsigned char value;
  if (cc2500_read_register(bp, CC2500_REG_MARCSTATE, &value) < 0)
    return -1;

  if (marcstate != NULL)
    *marcstate= value;
  return 0;
}

int cc2500_strobe(BP * bp, unsigned char strobe, unsigned char * status)
{
  if (bp_bin_spi_cs(bp, 0) < 0)
    return -1;

  unsigned char data[]= { strobe };
  if (bp_bin_spi_bulk(bp, data, 1) < 0)
    return -1;

  if (bp_bin_spi_cs(bp, 1) < 0)
    return -1;

  if (status != NULL)
    *status= data[0];
  return 0;
}

void cc2500_show_status(unsigned char status)
{
  static const char * state[]=
    { "IDLE", "RX", "TX", "FSTXON", "CALIBRATE",
      "SETTLING", "RXFIFO_OVERFLOW", "TXFIFO_UNDERFLOW" };
  printf("CC2500 status (%u):\n", status);
  printf("  chip ready: %s\n", (status & CC2500_STATUS_CHIP_RDY?"NO":"YES"));
  printf("  state     : %s\n", state[(status & 0x70) >> 4]);
  printf("  bytes avlb: %u\n", status & 0x0F);
}

void cc2500_show_marcstate(unsigned char marcstate)
{
  static const char * state[]=
    { "SLEEP", "IDLE", "XOFF", "VCOON_MC", "REGON_MC",
      "MANCAL", "VCOON", "REGON", "STARTCAL", "BWBOOST",
      "FS_LOCK", "IFADCON", "ENDCAL", "RX", "RX_END",
      "RX_RST", "TXRX_SWITCH", "RXFIFO_OVERFLOW",
      "FSTXON", "TX", "TX_END", "RXTX_SWITCH",
      "TXFIFO_UNDERFLOW" };
  marcstate&= 0x1F;
  assert(marcstate <= 22);
  assert(marcstate != 0x00); 
  assert(marcstate != 0x02); 
  if ((marcstate == 0x00) || (marcstate == 0x02) || (marcstate > 22))
    printf("  marc_state: impossible value (%u)!\n", marcstate);
  else 
    printf("  marc_state: %s\n", state[marcstate & 0x1F]);
}

void cc2500_show_mdmcfg4_3(unsigned char cfg4, unsigned char cfg3)
{
  unsigned char chanbw_e= (cfg4 & 0xC0) >> 6;
  unsigned char chanbw_m= (cfg4 & 0x30) >> 4;
  unsigned char drate_e= cfg4 & 0x0F;
  unsigned char drate_m= cfg3;
  printf("MDMCFG4\n");
  printf("  CHANBW_E: %u\n", chanbw_e);
  printf("  CHANBW_M: %u\n", chanbw_m);
  unsigned long bw_channel= FXOSC;
  bw_channel/= 8*(4+chanbw_m)*(1 << chanbw_e);
  printf("    -> BWchannel = %lu Hz\n", bw_channel);
  printf("  DRATE_E : %u\n", drate_e);
  printf("MDMCFG3\n");
  printf("  DRATE_M : %u\n", drate_m);
  unsigned long r_data= FXOSC;
  r_data*= (256+drate_m)*(1 << drate_e);
  r_data/= (1L << 28);
  printf("    -> Rdata = %lu baud\n", r_data);
}

void cc2500_show_mdmcfg2(unsigned char cfg2)
{
  static const char * mod_format_table[8]=
    {"2-FSK", "GFSK", "-", "OOK", "-", "-", "-", "MSK"};
  static const char * sync_mode_table[8]=
    { "No preamble/sync",
      "15/16 sync word bits detected",
      "16/16 sync word bits detected",
      "30/32 sync word bits detected",
      "No preamble/sync, carries-sense > threshold",
      "15/16 + carrier-sense > threshold",
      "16/16 + carrier-sense > threshold",
      "30/32 + carrier-sense > threshold" };
  unsigned char dem_dcfilt_off= (cfg2 & 0x80) >> 7;
  unsigned char mod_format= (cfg2 & 0x70) >> 4;
  unsigned char manchester_en= (cfg2 & 0x08) >> 3;
  unsigned char sync_mode= (cfg2 & 0x07);
  printf("MDMCFG2\n");
  printf("  DEM_DCFILT_OFF: %s\n", dem_dcfilt_off?"disable":"enable");
  printf("  MOD_FORMAT    : %s\n", mod_format_table[mod_format]);
  printf("  MANCHESTER_EN : %s\n", manchester_en?"enable":"disable");
  printf("  SYNC_MODE     : %s\n", sync_mode_table[sync_mode]);
}

void cc2500_show_mdmcfg1_0(unsigned char cfg1, unsigned char cfg0)
{
  static unsigned char num_preamble_table[8]=
    { 2, 3, 4, 6, 8, 12, 16, 24 };
  unsigned char fec_en= (cfg1 & 0x80) >> 7;
  unsigned char num_preamble= (cfg1 & 0x70) >> 4;
  unsigned char chanspc_e= cfg1 & 0x03;
  unsigned char chanspc_m= cfg0;
  printf("MDMCFG1\n");
  printf("  FEC_EN      : %s\n", fec_en?"enable":"disable");
  printf("  NUM_PREAMBLE: %u\n", num_preamble_table[num_preamble]);
  printf("  CHANSPC_E   : %u\n", chanspc_e);
  printf("MDMCFG0\n");
  printf("  CHANSPC_M: %u\n", chanspc_m);
  unsigned long delta_f_channel= FXOSC;
  delta_f_channel*= (256+chanspc_m)*(1 << chanspc_e);
  delta_f_channel/= (1 << 18);
  printf("    -> Delta f channel = %lu Hz\n", delta_f_channel);
}
