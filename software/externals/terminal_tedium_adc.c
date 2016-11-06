/***********************************************************************
 * This header file contains the mcp3208 Spi class definition.
 * Its main purpose is to communicate with the MCP3208 chip using
 * the userspace spidev facility.
 * The class contains four variables:
 * mode        -> defines the SPI mode used. In our case it is SPI_MODE_0.
 * bitsPerWord -> defines the bit width of the data transmitted.
 *        This is normally 8. Experimentation with other values
 *        didn't work for me
 * speed       -> Bus speed or SPI clock frequency. According to
 *                https://projects.drogon.net/understanding-spi-on-the-raspberry-pi/
 *            It can be only 0.5, 1, 2, 4, 8, 16, 32 MHz.
 *                Will use 1MHz for now and test it further.
 * spifd       -> file descriptor for the SPI device
 *
 * edit mxmxmx: adapted for mcp3208 / terminal tedium
 *
 * ****************************************************************************/
#include "m_pd.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifdef __arm__
    #include <sys/ioctl.h>
    #include <fcntl.h>
    #include <linux/spi/spidev.h>
#endif

static t_class *terminal_tedium_adc_class;

typedef struct _terminal_tedium_adc
{
    t_object x_obj;
    t_outlet *x_out1;
    t_outlet *x_out2;
    t_outlet *x_out3;
    t_outlet *x_out4;
    t_outlet *x_out5;
    t_outlet *x_out6;
    t_outlet *x_out7;
    t_outlet *x_out8;
    t_outlet *x_out9;
    unsigned char mode;
    unsigned char bitsPerWord;
    unsigned int speed;
    unsigned int smooth;
    unsigned int smooth_shift;
    unsigned int deadband;
    int spifd;
    int _version;
    int a2d[8];
} t_terminal_tedium_adc;

static t_terminal_tedium_adc *terminal_tedium_adc_new(t_floatarg version);
static int terminal_tedium_adc_write_read(t_terminal_tedium_adc *spi, unsigned char *data, int length);
static void terminal_tedium_adc_open(t_terminal_tedium_adc *spi, t_symbol *version);
static int terminal_tedium_adc_close(t_terminal_tedium_adc *spi);
static void terminal_tedium_adc_free(t_terminal_tedium_adc *spi);

/**********************************************************
 * terminal_tedium_adc_open() :function is called by the "open" command
 * It is responsible for opening the spidev device
 * and then setting up the spidev interface.
 * member variables are used to configure spidev.
 * They must be set appropriately before calling
 * this function.
 * *********************************************************/
static void terminal_tedium_adc_open(t_terminal_tedium_adc *spi, t_symbol *version){


    int statusVal = 0;
    
    if (strlen(version->s_name) == 0) spi->_version = 0x0; // = wm8731 version
    else spi->_version = 0x1; // = pcm5102a version

    #ifdef __arm__
      // we're using CS1 :
      spi->spifd =  open("/dev/spidev0.1", O_RDWR); 

      if(spi->spifd < 0) {
        statusVal = -1;
        pd_error(spi, "could not open SPI device");
        goto spi_output;
      }
   
      statusVal = ioctl (spi->spifd, SPI_IOC_WR_MODE, &(spi->mode));
      if(statusVal < 0){
        pd_error(spi, "Could not set SPIMode (WR)...ioctl fail");
        terminal_tedium_adc_close(spi);
        goto spi_output;
      }
   
      statusVal = ioctl (spi->spifd, SPI_IOC_RD_MODE, &(spi->mode));
      if(statusVal < 0) {
        pd_error(spi, "Could not set SPIMode (RD)...ioctl fail");
        terminal_tedium_adc_close(spi);
        goto spi_output;
      }
   
      statusVal = ioctl (spi->spifd, SPI_IOC_WR_BITS_PER_WORD, &(spi->bitsPerWord));
      if(statusVal < 0) {
        pd_error(spi, "Could not set SPI bitsPerWord (WR)...ioctl fail");
        terminal_tedium_adc_close(spi);
        goto spi_output;
      }
   
      statusVal = ioctl (spi->spifd, SPI_IOC_RD_BITS_PER_WORD, &(spi->bitsPerWord));
      if(statusVal < 0) {
        pd_error(spi, "Could not set SPI bitsPerWord(RD)...ioctl fail");
        terminal_tedium_adc_close(spi);
        goto spi_output;
      }  
   
      statusVal = ioctl (spi->spifd, SPI_IOC_WR_MAX_SPEED_HZ, &(spi->speed));    
      if(statusVal < 0) {
        pd_error(spi, "Could not set SPI speed (WR)...ioctl fail");
        terminal_tedium_adc_close(spi);
        goto spi_output;
      }
   
      statusVal = ioctl (spi->spifd, SPI_IOC_RD_MAX_SPEED_HZ, &(spi->speed));    
      if(statusVal < 0) {
        pd_error(spi, "Could not set SPI speed (RD)...ioctl fail");
        terminal_tedium_adc_close(spi);
        goto spi_output;
      }
   spi_output:
      if (!statusVal) statusVal = 1;
      else statusVal = 0;
      outlet_float(spi->x_out9, statusVal);  
 #endif
}

/***********************************************************
 * terminal_tedium_adc_close(): Responsible for closing the spidev interface.
 * *********************************************************/
 
static int terminal_tedium_adc_close(t_terminal_tedium_adc *spi){

    int statusVal = -1;
    if (spi->spifd == -1) {
      pd_error(spi, "terminal_tedium_adc: device not open");
      return(-1);
    }
  #ifdef __arm__ 
    statusVal = close(spi->spifd);
  #else 
    statusVal = 0x0;
  #endif  
    if(statusVal < 0) {
      pd_error(spi, "terminal_tedium_adc: could not close SPI device");
      exit(1);
    }
    outlet_float(spi->x_out9, 0);
    spi->spifd = -1;
    return(statusVal);
}

/***********************************************************
 * terminal_tedium_adc_smooth(): set smoothing
 * *********************************************************/
 
void terminal_tedium_adc_smooth(t_terminal_tedium_adc *spi, t_floatarg s){

    unsigned int _shift, _smooth = s;

    if (_smooth < 2) {
      _smooth = 1;
      _shift  = 0;
    }
    else if (_smooth < 4) {
      _smooth = 2;
      _shift  = 1;
    }
    else if (_smooth < 8) {
      _smooth = 4;
      _shift  = 2;
    }
    else if (_smooth < 16) {
      _smooth = 8;
      _shift  = 3;
    }
    else {
      _smooth = 16;  
      _shift  = 4; 
    }
   
    spi->smooth = _smooth;
    spi->smooth_shift = _shift;
}

/***********************************************************
 * terminal_tedium_adc_deadband(): set deadband
 * *********************************************************/
 
void terminal_tedium_adc_deadband(t_terminal_tedium_adc *spi, t_floatarg d){

    int _deadband = (int)d;

    if (_deadband < 0)
      _deadband = 0;
    else if (_deadband > 5)
      _deadband = 5; 
   
    spi->deadband = _deadband;
}

/********************************************************************
 * This function frees the object (destructor).
 * ******************************************************************/
static void terminal_tedium_adc_free(t_terminal_tedium_adc *spi){
    if (spi->spifd == 0) {
      terminal_tedium_adc_close(spi);
    }
}
 
/********************************************************************
 * This function writes data "data" of length "length" to the spidev
 * device. Data shifted in from the spidev device is saved back into
 * "data".
 * ******************************************************************/
static int terminal_tedium_adc_write_read(t_terminal_tedium_adc *spi, unsigned char *data, int length){
 
  #ifdef __arm__ 

    struct spi_ioc_transfer spid[length];
    int i = 0;
    int retVal = -1; 

 
  // one spi transfer for each byte
    for (i = 0 ; i < length ; i++){
      
        memset (&spid[i], 0x0, sizeof (spid[i]));
        spid[i].tx_buf        = (unsigned long)(data + i); // transmit from "data"
        spid[i].rx_buf        = (unsigned long)(data + i); // receive into "data"
        spid[i].len           = sizeof(*(data + i));
        spid[i].speed_hz      = spi->speed;
        spid[i].bits_per_word = spi->bitsPerWord;
    }
 
    retVal = ioctl(spi->spifd, SPI_IOC_MESSAGE(length), &spid);

    if(retVal < 0){
       pd_error(spi, "problem transmitting spi data..ioctl");
    }
  #else // dummy crap
    int retVal = length + spi->speed + sizeof(*(data));
  #endif
    return retVal;
}

/***********************************************************************
 * mcp3208 enabled external that by default interacts with /dev/spidev0.0 device using
 * terminal_tedium_adc_MODE_0 (MODE 0) (defined in linux/spi/spidev.h), speed = 1MHz &
 * bitsPerWord=8.
 *
 * *********************************************************************/
 
static void terminal_tedium_adc_bang(t_terminal_tedium_adc *spi)
{
  #ifdef __arm__ 
    if (spi->spifd == -1) {
        pd_error(spi, "device not open %d", spi->spifd);
        return;
  }
  #else 
      return;
  #endif

  int a2dVal[8] = {0,0,0,0,0,0,0,0};
  int a2dChannel = 0;
  unsigned char data[3];
  int numChannels = spi->_version ? 0x8 : 0x6 ; // 8 channels for pcm5102a, 6 channels for wm8731 version
  int SCALE = 4001; // make nice zeros ...

  unsigned int SMOOTH = spi->smooth;
  unsigned int SMOOTH_SHIFT = spi->smooth_shift;
  int DEADBAND = spi->deadband;

  
  for (unsigned int i = 0; i < SMOOTH; i++) {

      for (a2dChannel = 0; a2dChannel < numChannels; a2dChannel++) {

        data[0]  =  0x06 | ((a2dChannel>>2) & 0x01);
        data[1]  =  a2dChannel<<6;
        data[2]  =  0x00;

        terminal_tedium_adc_write_read(spi, data, 3);

        a2dVal[a2dChannel] += (((data[1] & 0x0f) << 0x08) | data[2]); 
      }
  }

  for (a2dChannel = 0; a2dChannel < numChannels; a2dChannel++) {

      if (DEADBAND) {

        int tmp  = SCALE - (a2dVal[a2dChannel] >> SMOOTH_SHIFT);
        int tmp2 = spi->a2d[a2dChannel];

        if ((tmp2 - tmp) > DEADBAND || (tmp - tmp2) > DEADBAND)  
          a2dVal[a2dChannel] = tmp < 0 ? 0 : tmp;
        else 
          a2dVal[a2dChannel] = tmp2;
        spi->a2d[a2dChannel] = a2dVal[a2dChannel];
      }
      else {

        int tmp = SCALE - (a2dVal[a2dChannel] >> SMOOTH_SHIFT);
        a2dVal[a2dChannel] = tmp < 0 ? 0 : tmp;
      }
  }
 
  if (numChannels < 0x7) {  // wm8731 :
     
      outlet_float(spi->x_out6, a2dVal[5]);
      outlet_float(spi->x_out5, a2dVal[4]);
      outlet_float(spi->x_out4, a2dVal[3]);
      outlet_float(spi->x_out3, a2dVal[2]);
      outlet_float(spi->x_out2, a2dVal[1]);
      outlet_float(spi->x_out1, a2dVal[0]);
  }
  else { // map to panel (pcm5102a): 
      outlet_float(spi->x_out8, a2dVal[2]);
      outlet_float(spi->x_out7, a2dVal[3]);
      outlet_float(spi->x_out6, a2dVal[0]);
      outlet_float(spi->x_out5, a2dVal[7]);
      outlet_float(spi->x_out4, a2dVal[4]);
      outlet_float(spi->x_out3, a2dVal[1]);
      outlet_float(spi->x_out2, a2dVal[6]);
      outlet_float(spi->x_out1, a2dVal[5]);
  }
}

/*************************************************
 * init function.
 * ***********************************************/
static t_terminal_tedium_adc *terminal_tedium_adc_new(t_floatarg version){
    t_terminal_tedium_adc *spi = (t_terminal_tedium_adc *)pd_new(terminal_tedium_adc_class);

    spi->x_out1 = outlet_new(&spi->x_obj, gensym("float"));
    spi->x_out2 = outlet_new(&spi->x_obj, gensym("float"));
    spi->x_out3 = outlet_new(&spi->x_obj, gensym("float"));
    spi->x_out4 = outlet_new(&spi->x_obj, gensym("float"));
    spi->x_out5 = outlet_new(&spi->x_obj, gensym("float"));
    spi->x_out6 = outlet_new(&spi->x_obj, gensym("float"));
    spi->x_out7 = outlet_new(&spi->x_obj, gensym("float"));
    spi->x_out8 = outlet_new(&spi->x_obj, gensym("float"));
    spi->x_out9 = outlet_new(&spi->x_obj, gensym("float"));
    #ifdef __arm__ 
      spi->mode = SPI_MODE_0;
    #else
      spi->mode = 0x0;
    #endif   
    spi->bitsPerWord = 8;
    spi->speed = 4000000;
    spi->spifd = -1;
    spi->_version = version;
    spi->smooth = 1; 
    spi->smooth_shift = 0;
    spi->deadband = 0;
    return(spi);
}


void terminal_tedium_adc_setup(void)
{
    terminal_tedium_adc_class = class_new(gensym("terminal_tedium_adc"), (t_newmethod)terminal_tedium_adc_new,
        (t_method)terminal_tedium_adc_free, sizeof(t_terminal_tedium_adc), 0, A_DEFSYM, 0);
    class_addmethod(terminal_tedium_adc_class, (t_method)terminal_tedium_adc_open, gensym("open"), 
        A_DEFSYM, 0);
    class_addmethod(terminal_tedium_adc_class, (t_method)terminal_tedium_adc_close, gensym("close"), 
        0, 0);
    class_addmethod(terminal_tedium_adc_class, (t_method)terminal_tedium_adc_smooth, gensym("smooth"), 
        A_DEFFLOAT, 0);
    class_addmethod(terminal_tedium_adc_class, (t_method)terminal_tedium_adc_deadband, gensym("deadband"), 
        A_DEFFLOAT, 0);
    class_addbang(terminal_tedium_adc_class, terminal_tedium_adc_bang);
}
