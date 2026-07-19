#include <assert.h>
#include <csignal>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "bus.h"
#include "spi.h"

#include "24c.h"
#include "24f.h"
#include "cc.h"
#include "ds.h"
#include "mr.h"


void show_pins_state(unsigned char value, int power, int pullup)
{
  printf("  Pin CS  : %s\n", (value & BP_PIN_CS)?"ON":"OFF");
  printf("  Pin MISO: %s\n", (value & BP_PIN_MISO)?"ON":"OFF");
  printf("  Pin CLK : %s\n", (value & BP_PIN_CLK)?"ON":"OFF");
  printf("  Pin MOSI: %s\n", (value & BP_PIN_MOSI)?"ON":"OFF");
  printf("  Pin AUX : %s\n", (value & BP_PIN_AUX)?"ON":"OFF");
  if (pullup)
    printf("  Pullup  : %s\n", (value & BP_PIN_PULLUP)?"ON":"OFF");
  if (power)
    printf("  Power   : %s\n", (value & BP_PIN_POWER)?"ON":"OFF");
}

void show_spi_speed(unsigned char speed)
{
  assert(speed <= 7);
  static char * speeds[8]= {"30kHz", "125kHz", "250kHz", "1MHz",
			  "2MHz", "2.6MHz", "4MHz", "8MHz"};
  printf("  Speed: %s\n", speeds[speed]);
}

void show_spi_config(unsigned char config)
{
  assert(config <= 15);
  printf("  Output level   : %s\n",
	 (config & BP_BIN_SPI_LV_3V3?"3V3":"HiZ"));
  printf("  Clock idle     : %s\n",
	 (config & BP_BIN_SPI_CLK_IDLE_HIGH?"high":"low"));
  printf("  Clock edge     : %s\n",
	 (config & BP_BIN_SPI_CLK_EDGE_HIGH?"idle->active":"active->idle"));
  printf("  Sample position: %s\n",
	 (config & BP_BIN_SPI_SMP_END?"end":"middle"));
}

int demo_bitbang(BP * bp)
{
  unsigned char value;

  printf("*** bitbang demo ***\n");

  printf("Setup pins as output\n");
  if (bp_bin_pins_setup(bp, 0, 0, 0, 0, 0, &value) < 0)
    return -1;
  show_pins_state(value, 0, 0);

  printf("Set pins to ON\n");
  if (bp_bin_pins_set(bp, 1, 1, 1, 1, 1, 1, 1, &value) < 0)
    return -1;
  show_pins_state(value, 1, 1);

  return 0;
}

int demo_spi(BP * bp)
{
  unsigned char version;
  printf("*** SPI demo ***\n");
  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) < 0)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  if (bp_bin_spi_version(bp, &version) < 0)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  unsigned char speed;
  printf("Reading SPI speed\n");
  if (bp_bin_spi_get_speed(bp, &speed) < 0)
    return -1;
  printf("Binary I/O SPI speed:\n");
  show_spi_speed(speed);

  printf("Setting SPI speed\n");
  if (bp_bin_spi_set_speed(bp, BP_BIN_SPI_SPEED_8M) < 0)
    return -1;
  printf("Reading SPI speed\n");
  if (bp_bin_spi_get_speed(bp, &speed) < 0)
    return -1;
  printf("Binary I/O SPI speed:\n");
  show_spi_speed(speed);

  unsigned char config;
  printf("Reading SPI config\n");
  if (bp_bin_spi_get_config(bp, &config) < 0)
    return -1;
  printf("Binary I/O SPI config (%u):\n", config);
  show_spi_config(config);

  printf("Setting SPI config\n");
  if (bp_bin_spi_set_config(bp, BP_BIN_SPI_LV_HIZ |
			    BP_BIN_SPI_CLK_IDLE_LOW |
			    BP_BIN_SPI_CLK_EDGE_HIGH |
			    BP_BIN_SPI_SMP_MIDDLE) < 0)
    return -1;

  printf("Reading SPI config\n");
  if (bp_bin_spi_get_config(bp, &config) < 0)
    return -1;
  printf("Binary I/O SPI config (%u):\n", config);
  show_spi_config(config);

  int i;
  for (i= 0; i < 10; i++) {
    printf("Enabling CS\n");
    if (bp_bin_spi_cs(bp, 1) < 0)
      return -1;
    usleep(100000);
    printf("Disabling CS\n");
    if (bp_bin_spi_cs(bp, 0) < 0)
      return -1;
    usleep(100000);
  }
  return 0;
}

void show_data(const char * msg, const unsigned char * data,
	       unsigned char nbytes)
{
  int i;
  printf("%s", msg);
  for (i= 0; i < nbytes; i++)
    printf(" 0x%.2X", data[i]);
  printf("\n");
}

int demo_spi_cc2500(BP * bp)
{
  unsigned char version, config;
  printf("CC2500 SPI demo ***\n");
  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) < 0)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  printf("Setting SPI speed to 1MHz\n");
  if (bp_bin_spi_set_speed(bp, BP_BIN_SPI_SPEED_1M) < 0)
    return -1;

  printf("Setting SPI config (open-drain,clk-idle=LOW,clk-edge=i->a,smp=middle)\n");
  if (bp_bin_spi_set_config(bp, BP_BIN_SPI_LV_HIZ |
			    BP_BIN_SPI_CLK_IDLE_LOW |
			    BP_BIN_SPI_CLK_EDGE_LOW |
			    BP_BIN_SPI_SMP_MIDDLE) < 0)
    return -1;
  printf("Reading SPI config\n");
  if (bp_bin_spi_get_config(bp, &config) < 0)
    return -1;
  printf("Binary I/O SPI config (%u):\n", config);
  show_spi_config(config);

  usleep(1000);

  printf("Setting SPI periph config (power=ON,pullups=ON)\n");
  if (bp_bin_spi_set_periph(bp, 0x08 | 0x04) < 0)
    return -1;

  usleep(1000000);

  printf("Reading SPI periph config\n");
  if (bp_bin_spi_get_periph(bp, &config) < 0)
    return -1;
  printf("  config=%u\n", config);

  unsigned long freq;
  if (cc2500_get_freq(bp, &freq) < 0)
    return -1;
  printf("Frequency: %lu\n", freq);

  unsigned char channel;
  if (cc2500_get_channel(bp, &channel) < 0)
    return -1;
  printf("Channel: %u\n", channel);

  unsigned char cc2500_version, cc2500_partnum;
  cc2500_get_version(bp, &cc2500_version, &cc2500_partnum);
  if ((cc2500_version != 0x03) || (cc2500_partnum != 0x80)) {
    printf("Error: unexpected CC2500 version/partnum\n");
    return -1;
  }
  printf("Version: %u\n", cc2500_version);
  printf("Partnum: %u\n", cc2500_partnum);

  unsigned char marcstate;
  if (cc2500_get_marcstate(bp, &marcstate) < 0)
    return -1;
  cc2500_show_marcstate(marcstate);

  unsigned char status;
  if (cc2500_strobe(bp, CC2500_SRX, &status) < 0)
    return -1;
  cc2500_show_status(status);

  if (cc2500_get_marcstate(bp, &marcstate) < 0)
    return -1;
  cc2500_show_marcstate(marcstate);

  unsigned char mdmcfg4, mdmcfg3;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG4, &mdmcfg4) < 0)
    return -1;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG3, &mdmcfg3) < 0)
    return -1;
  cc2500_show_mdmcfg4_3(mdmcfg4, mdmcfg3);
  unsigned char value;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG2, &value) < 0)
    return -1;
  cc2500_show_mdmcfg2(value);
  unsigned char mdmcfg1, mdmcfg0;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG1, &mdmcfg1) < 0)
    return -1;
  if (cc2500_read_register(bp, 0x80 + CC2500_REG_MDMCFG0, &mdmcfg0) < 0)
    return -1;
  cc2500_show_mdmcfg1_0(mdmcfg1, mdmcfg0);
  return 0;
}

int demo_spi_mrf24j40_send(BP * bp) 
{
  unsigned char version;
  unsigned char config;

  printf("*** MRF24J40 send SPI demo ***\n");
  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) < 0)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  printf("Setting SPI speed to 30KHz\n");
  if (bp_bin_spi_set_speed(bp, BP_BIN_SPI_SPEED_1M) < 0)
    return -1;

  printf("Reading SPI config\n");
  if (bp_bin_spi_get_config(bp, &config) < 0)
    return -1;
  printf("Binary I/O SPI config (%u):\n", config);
  show_spi_config(config);

  printf("Setting PAN ID (low)\n");

      config= BP_BIN_SPI_LV_HIZ;
      printf("Binary I/O SPI config (%u):\n", config);
      if (bp_bin_spi_set_config(bp, config) < 0)
	return -1;

  printf("Setting SPI config (open-drain,clk-idle=LOW,clk-edge=i->a,smp=middle)\n");
  if (bp_bin_spi_set_config(bp, BP_BIN_SPI_LV_HIZ |
			    BP_BIN_SPI_CLK_IDLE_LOW |
			    BP_BIN_SPI_CLK_EDGE_LOW |
			    BP_BIN_SPI_SMP_MIDDLE) < 0)
    return -1;
  printf("Reading SPI config\n");
  if (bp_bin_spi_get_config(bp, &config) < 0)
    return -1;
  printf("Binary I/O SPI config (%u):\n", config);
  show_spi_config(config);
  usleep(1000);

  printf("Setting SPI periph config (power=ON,pullups=ON)\n");
  if (bp_bin_spi_set_periph(bp, 0x08 | 0x04) < 0)
    return -1;

  usleep(100000);

  printf("Reading SPI periph config\n");
  if (bp_bin_spi_get_periph(bp, &config) < 0)
    return -1;
  printf("  config =%u\n", config);
  printf("  power  =%s\n", (config & 0x08)?"ON":"OFF");
  printf("  pullups=%s\n", (config & 0x04)?"ON":"OFF");

  usleep(100000);

  printf("Hit enter to continue...");
  getchar();
  
  if (mrf24j40_xtalk_test(bp, 500) != 0)
    return -1;

  unsigned char mac[]= {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 };
  if (mrf24j40_init(bp, 0x0001, 0x0001, mac, 11, 1) < 0)
    return -1;

  int i;
  for (i= 0; i < 3; i++) {
    int res= mrf24j40_tx(bp, 0x0001, 0x0001, 0x0002, i, 1, NULL, 0);
    if (res < 0)
      return -1;
    if (res == 0)
      printf("Transmission failed, retry count exceeded :-(\n");
    else
      printf("Transmission successful :-)\n");
    usleep(1000000);
  }
  return 0;
}

int loop= 1;
static void signal_handler(int signum) {
  fprintf(stdout, "Ctrl-C received\n");
  fflush(stdout);
  loop= 0;
}

int demo_spi_mrf24j40_recv(BP * bp) {
  unsigned char version;
  unsigned char config;

  printf("*** MRF24J40 recv SPI demo ***\n");

  printf("Entering SPI mode\n");
  if (bp_bin_mode_spi(bp, &version) < 0)
    return -1;
  printf("Binary I/O SPI version: %u\n", version);

  printf("Setting SPI speed to 30KHz\n");
  if (bp_bin_spi_set_speed(bp, BP_BIN_SPI_SPEED_1M) < 0)
    return -1;

  printf("Reading SPI config\n");
  if (bp_bin_spi_get_config(bp, &config) < 0)
    return -1;
  printf("Binary I/O SPI config (%u):\n", config);
  show_spi_config(config);

  printf("Setting PAN ID (low)\n");

      config= BP_BIN_SPI_LV_HIZ;
      printf("Binary I/O SPI config (%u):\n", config);
      if (bp_bin_spi_set_config(bp, config) < 0)
	return -1;

  printf("Setting SPI config (open-drain,clk-idle=LOW,clk-edge=i->a,smp=middle)\n");
  if (bp_bin_spi_set_config(bp, BP_BIN_SPI_LV_HIZ |
			    BP_BIN_SPI_CLK_IDLE_LOW |
			    BP_BIN_SPI_CLK_EDGE_LOW |
			    BP_BIN_SPI_SMP_MIDDLE) < 0)
    return -1;
  printf("Reading SPI config\n");
  if (bp_bin_spi_get_config(bp, &config) < 0)
    return -1;
  printf("Binary I/O SPI config (%u):\n", config);
  show_spi_config(config);
  usleep(1000);

  printf("Setting SPI periph config (power=ON,pullups=ON)\n");
  if (bp_bin_spi_set_periph(bp, 0x08 | 0x04) < 0)
    return -1;

  usleep(100000);

  printf("Reading SPI periph config\n");
  if (bp_bin_spi_get_periph(bp, &config) < 0)
    return -1;
  printf("  config =%u\n", config);
  printf("  power  =%s\n", (config & 0x08)?"ON":"OFF");
  printf("  pullups=%s\n", (config & 0x04)?"ON":"OFF");

  usleep(100000);

  printf("Hit enter to continue...");
  getchar();

  unsigned char ladr[]= {0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89};
  if (mrf24j40_init(bp, 0x0001, 0x0002, ladr, 11, 1) < 0)
    return -1;

  if (mrf24j40_promiscuous(bp, 0) < 0)
    return -1;

  mac_frame frame;
  unsigned char rssi, lqi;
  while (loop) {
    int res= mrf24j40_rx(bp, &frame, &rssi, &lqi);
    if (res < 0)
      return -1;
    if (res == 1) {
      printf("A packet has been received :-)\n");
      printf(" RSSI=%u\n", rssi);
      printf(" LQI =%u\n", lqi);
      mac_frame_dump(stdout, &frame);
    } else
      usleep(50000);
  }

  return 0;
}

typedef struct demo_t {
  char * name;
  int (* fct)(BP * bp);
} demo_t;

demo_t demos[]=
  {
    { "bitbang",           demo_bitbang },
    { "spi",               demo_spi },
    { "spi-cc2500",        demo_spi_cc2500},
    { "spi-mrj24j40-send", demo_spi_mrf24j40_send},
    { "spi-mrj24j40-recv", demo_spi_mrf24j40_recv},
    { "i2c-ds1624",        ds1624_demo},
    { "i2c-24c02",         _24c_demo},
    { "i2c-24fc1025",      _24fc1025_demo},
  };
#define NUM_DEMOS sizeof(demos)/sizeof(demo_t)

void help(const char * msg, ...)
{
  va_list ap;
  int i;

  if (msg != NULL) {
    va_start(ap, msg);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, msg, ap);
    va_end(ap);
  }

  fprintf(stderr, "\n\n");
  fprintf(stderr, "usage: jbus device demo-id\n");
  fprintf(stderr, "\n");

  for (i= 0; i < NUM_DEMOS; i++)
    fprintf(stderr, "  %.2u %%\n", i+1);

  if (msg != NULL)
    exit(EXIT_FAILURE);
}

int main(int argc, char * argv[])
{
  BP * bp;
  unsigned char version;
  int demonum= 0;

  if (argc < 3)
    help("not enough arguments");

  demonum= atoi(argv[2]);
  if ((demonum < 1) || (demonum > NUM_DEMOS+1))
    help("invalid demo-id");
  assert(signal(SIGINT, signal_handler) != SIG_ERR);

  bp= bp_open(argv[1]);
  if (bp == NULL) {
    fprintf(stderr, "Could not open bus pirate\n");
    goto fail;
  }

  if (bp_bin_init(bp, &version) < 0)
    goto fail;
  printf("Binary I/O mode version: %u\n", version);

  if (demos[demonum-1].fct(bp) < 0)
    goto fail;

  printf("Reset binary mode\n");
  if (bp_bin_reset(bp, NULL) < 0)
    goto fail;
  printf("Closing.\n");
  bp_close(bp);
  exit(EXIT_SUCCESS);

 fail:
  fprintf(stderr, "Exit with error\n");
  if (bp != NULL)
    bp_close(bp);
  exit(EXIT_FAILURE);
}
