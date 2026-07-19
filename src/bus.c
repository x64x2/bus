#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "bus.h"

#define BUFFER_SIZE 1024
#define TIMEOUT_MS   5
#define NRETRIES     3

struct BP_t 
{
  int            fd;
  struct termios saved_tios;
  unsigned char  buffer[BUFFER_SIZE];
  int            buffer_numc;
  int            buffer_pos;
  int            state;
} BP_t;


BP * bp_open(const char * filename)
{
  struct termios tios;
  BP * bp= malloc(sizeof(BP));
  assert(bp != NULL);

  bp->fd= open(filename, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (bp->fd < 0) {
    perror("open");
    goto fail;
  }

  bp->buffer_numc= 0;
  bp->buffer_pos= 0;
  bp->state= BP_STATE_TEXT;

  if (tcgetattr(bp->fd, &tios) < 0) {
    perror("tcgetattr");
    goto fail;
  }
  if (tcgetattr(bp->fd, &bp->saved_tios) < 0) {
    perror("tcgetattr");
    goto fail;
  }
  tios.c_cflag= CS8 | CLOCAL | CREAD;
  tios.c_iflag= IGNPAR | BRKINT;
  tios.c_oflag= 0;
  tios.c_lflag= 0;
  if (cfsetspeed(&tios, B115200) < 0) {
    perror("cfsetspeed");
    goto fail;
  }
  if (tcsetattr(bp->fd, TCSANOW, &tios) < 0) {
    perror("tcsetattr");
    goto fail;
  }

  return bp;

 fail:
  if (bp != NULL) {
    if (bp->fd >= 0)
      close(bp->fd);
    free(bp);
  }
  return NULL;
}

void bp_close(BP * bp)
{
  assert(bp != NULL);
  assert(bp->fd >= 0);
  
  tcsetattr(bp->fd, TCSAFLUSH, &bp->saved_tios);

  if (close(bp->fd) < 0) {
    perror("close");
    exit(EXIT_FAILURE);
  }

  bp->fd= -1;

  free(bp);
}

void _bp_check_state(BP * bp, int state)
{
  assert(bp != NULL);
  assert(bp->state == state);
}

void _bp_flush(BP * bp)
{
  tcflush(bp->fd, TCIOFLUSH);
  bp->buffer_numc= 0;
  bp->buffer_pos= 0;
}

int bp_readc(BP * bp, unsigned char * c)
{
  fd_set rset;
  struct timeval tv;

  assert(bp != NULL);
  assert(bp->fd >= 0);

  if (bp->buffer_numc < 1) {
    bp->buffer_pos= 0;
    bp->buffer_numc= 0;

    int nretries= NRETRIES;
    do {
      FD_ZERO(&rset);
      FD_SET(bp->fd, &rset);
      tv.tv_sec= 0;
      tv.tv_usec= TIMEOUT_MS*1000;
      int res;
      if ((res= select(bp->fd+1, &rset, NULL, NULL, &tv)) < 0) {
	perror("select");
	return -1;
      }
      nretries--;
    } while ((nretries > 0) && !FD_ISSET(bp->fd, &rset));

    if (!FD_ISSET(bp->fd, &rset)) {
      return -1;
    }


    bp->buffer_numc= read(bp->fd, bp->buffer, sizeof(bp->buffer));
    if (bp->buffer_numc < 0) {
      perror("  read");
      return -1;
    }
    assert(bp->buffer_numc != 0);
    int i;

    for (i= 0; i < bp->buffer_numc; i++)
    
    if (bp->buffer_numc > 0) 
    {
      *c= bp->buffer[bp->buffer_pos];
      bp->buffer_pos++;
      bp->buffer_numc-= 1;
      return 1;
    }
 }
 return 0;
}

int bp_write(BP * bp, const void * buf, size_t nbyte)
{
  int i;
  for (i= 0; i < nbyte; i++)
  assert(bp != NULL);
  assert(bp->fd >= 0);

  return write(bp->fd, buf, nbyte);
}

int _bp_bin_write_read(BP * bp, unsigned char cmd,
		       unsigned char * buf, size_t buflen)
{
  if (bp_write(bp, &cmd, 1) != 1)
    return -1;
  if (bp_read(bp, buf, buflen) != buflen)
    return -1;
  return 0;
}

int bp_bin_reset(BP * bp, unsigned char * version)
{
  unsigned char rbuf[5];
  assert(bp != NULL);

  if (_bp_bin_write_read(bp, BP_BIN_RESET, rbuf, 5) < 0)
    return -1;
  if (memcmp(rbuf, "BBIO", 4) != 0)
    return -1;
  if ((rbuf[4] < '1') || (rbuf[4] > '9'))
    return -1;
  if (version != NULL)
    *version= rbuf[4]-'0';
  return 0;
}

int bp_bin_init(BP * bp, unsigned char * version)
{
  assert(bp != NULL);
  unsigned char vers= 0;
  int nretries= 20; 

  if (bp->state == BP_STATE_TEXT) {

    _bp_flush(bp);

    do {
      if (bp_write(bp, "\0", 1) != 1)
	return -1;
      unsigned char buf[5];
      if (bp_read(bp, buf, sizeof(buf)) == 5) {
	if (memcmp(buf, "BBIO", 4) != 0)
	  return -1;
	if ((buf[4] < '1') || (buf[4] > '9'))
	  return -1;
	vers= buf[4]-'0';

	assert(vers == 1);

	break;
      }
     
      nretries--;
    } while (nretries > 0);
      
  }

  _bp_flush(bp);

  if (version != NULL)
    *version= vers;
  bp->state= BP_STATE_BIN;
  return 0;
}

int bp_bin_pins_setup(BP * bp, int aux, int mosi, int clk, int miso, int cs,
		      unsigned char * value)
{
  assert(bp != NULL);
  assert(bp->state == BP_STATE_BIN);

  unsigned char data= BP_BIN_PINS_SETUP;
  if (cs)   data|= BP_PIN_CS;
  if (miso) data|= BP_PIN_MISO;
  if (clk)  data|= BP_PIN_CLK;
  if (mosi) data|= BP_PIN_MOSI;
  if (aux)  data|= BP_PIN_AUX;

  if (bp_write(bp, &data, 1) != 1)
    return -1;

  unsigned char buf;
  if (bp_readc(bp, &buf) != 1)
    return -1;

  /* Check that 3 MSB of returned value correspond to 3 MSB of sent
     value. */
  assert((buf & 0xE0) == 0x40);

  if (value != NULL)
    *value= buf & 0x1F;

  return 0;
}

int bp_bin_pins_set(BP * bp, int power, int pullup, int aux, int mosi, int clk, int miso, int cs, unsigned char * value)
{
  assert(bp != NULL);
  assert(bp->state == BP_STATE_BIN);

  unsigned char data= BP_BIN_PINS_SET;
  if (cs)     data|= BP_PIN_CS;
  if (miso)   data|= BP_PIN_MISO;
  if (clk)    data|= BP_PIN_CLK;
  if (mosi)   data|= BP_PIN_MOSI;
  if (aux)    data|= BP_PIN_AUX;
  if (pullup) data|= BP_PIN_PULLUP;
  if (power)  data|= BP_PIN_POWER;

  if (bp_write(bp, &data, 1) != 1)
    return -1;

  unsigned char buf;
  if (bp_readc(bp, &buf) != 1)
    return -1;

  /* Check that returned value corresponds for 'power' and 'pullup'.
     The value of other pins might differ if they were configured as
     input (see bp_bin_pins_setup). */
  assert((buf & 0xE0) == (data & 0xE0));

  if (value != NULL)
    *value= buf & 0x7F;

  return 0;
}

int  bp_bin_mode_i2c(BP * bp, unsigned char * version)
{
  assert(bp != NULL);
  assert(bp->state == BP_STATE_BIN);

  unsigned char data= BP_BIN_I2C;
  if (bp_write(bp, &data, 1) != 1)
    return -1;

  unsigned char rbuf[4];
  if (bp_read(bp, rbuf, 4) != 4)
    return -1;
  if (memcmp(rbuf, "I2C", 3) != 0)
    return -1;
  assert((rbuf[3] >= '1') && (rbuf[3] <= '9'));
  if (version != NULL)
    *version= rbuf[3]-'0';

  bp->state= BP_STATE_BIN_I2C;
  return 0;
}

int bp_bin_mode_spi(BP * bp, unsigned char * version)
{
  assert(bp != NULL);
  assert(bp->state == BP_STATE_BIN);

  unsigned char data= BP_BIN_SPI;
  if (bp_write(bp, &data, 1) != 1)
    return -1;

  unsigned char rbuf[4];
  if (bp_read(bp, rbuf, 4) != 4)
    return -1;
  if (memcmp(rbuf, "SPI", 3) != 0)
    return -1;
  assert((rbuf[3] >= '1') && (rbuf[3] <= '9'));
  if (version != NULL)
    *version= rbuf[3]-'0';

  bp->state= BP_STATE_BIN_SPI;
  return 0;
}
