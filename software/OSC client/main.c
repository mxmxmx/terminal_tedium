/*
*
*   terminal tedium / OSC client (UDP)
*
*     
*   - TD: gate outputs (?); long press; make nicer. etc
*   - TD: proper timing / get rid of counters
*   - TD: make more userfriendly, pass IP and port as arguments, etc.
*
*   compile with: gcc *.c -Werror -lwiringPi -std=c99 -O2 -g -o tedium_osc 
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "tinyosc.h"

const uint16_t BUFLEN = 1024;
const uint64_t NOW = 1L;
const uint32_t TIMEOUT = 1000000;  // count up to timeout / ADC is read every .. 
const uint32_t TIMEOUT_S = 5;      // short time out (for OSC off messages) (buttons)
const uint32_t TIMEOUT_T = 1000;   // short time out (for OSC off messages) (triggers)

#define ADC_SPI_CHANNEL 1
#define ADC_SPI_SPEED 4000000
#define ADC_NUM_CHANNELS 6
#define RESOLUTION 4095
#define DEADBAND 2

// GPIO : 

#define B1 23
#define B2 25
#define B3 24
#define LED 26
#define TR1 4
#define TR2 17
#define TR3 14
#define TR4 27
#define GATE1 16
#define GATE2 12

void die(char *s)
{
    perror(s);
    exit(1);
}


uint16_t adc[ADC_NUM_CHANNELS]; // adc values


/*  interrupt flags etc:  */

static volatile uint8_t TR1_flag = 0;
static volatile uint8_t TR2_flag = 0;
static volatile uint8_t TR3_flag = 0;
static volatile uint8_t TR4_flag = 0;
static volatile uint8_t B1_flag  = 0;
static volatile uint8_t B2_flag  = 0;
static volatile uint8_t B3_flag  = 0;

void Interrupt_TR1 (void) { TR1_flag = 1; }
void Interrupt_TR2 (void) { TR2_flag = 1; }
void Interrupt_TR3 (void) { TR3_flag = 1; }
void Interrupt_TR4 (void) { TR4_flag = 1; }
void Interrupt_B1  (void) { B1_flag  = 1; }
void Interrupt_B2  (void) { B2_flag  = 1; }
void Interrupt_B3  (void) { B3_flag  = 1; }

// flags and counters for on/off messages:

uint8_t B1_OFF = 0;  
uint8_t B2_OFF = 0; 
uint16_t _wait_B1, _wait_B2; 

uint8_t TR1_OFF = 0; 
uint8_t TR2_OFF = 0; 
uint8_t TR3_OFF = 0; 
uint8_t TR4_OFF = 0; 
uint16_t _wait_TR1, _wait_TR2, _wait_TR3, _wait_TR4; 

/* 

mcp3208 : return 1 if we need to send OSC, 0 otherwise

*/

uint16_t readADC(int _channel, uint16_t *adc_val){ 

      uint8_t spi_data[3];
      uint16_t result, tmp = *(adc_val + _channel); // previous.

      spi_data[0] = 0x06 | (_channel>>2) & 0x01;    // single ended
      spi_data[1] = _channel<<6;
      spi_data[2] = 0x00;

      wiringPiSPIDataRW(ADC_SPI_CHANNEL, spi_data, 3);

      // invert + limit result: 
      result = 4000 - (((spi_data[1] & 0x0f) << 8) | spi_data[2]);
      result = result > RESOLUTION ? 0 : result;  
      
      if ( (result - tmp) > DEADBAND || (tmp - result) > DEADBAND ) { 
        *(adc_val + _channel) = result ;
        return 1;
      }

      else {
        *(adc_val + _channel)  = tmp;
        return 0;
      }
}

/* --------------------------------------------------------- */

int main(void)
{

    struct sockaddr_in _serv;
    int s, i, slen = sizeof(_serv);
    char buf[BUFLEN];
    int port = 9000;
    char *ip = "192.168.0.18";

    memset(adc, 0, ADC_NUM_CHANNELS);

    // open socket : 
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) die("socket");
    
    memset((char *) &_serv, 0, sizeof(_serv));
    _serv.sin_family = AF_INET;
    _serv.sin_port = htons(port);
    _serv.sin_addr.s_addr =  inet_addr(ip);

    // OSC bundle
    tosc_bundle bundle;

    // setup SPI
    wiringPiSetupGpio();
    wiringPiSPISetup(ADC_SPI_CHANNEL, ADC_SPI_SPEED);

    // pull ups:
    pinMode(B1, INPUT);
    pullUpDnControl(B1, PUD_UP);
    pinMode(B2, INPUT);
    pullUpDnControl(B2, PUD_UP);
    pinMode(B3, INPUT);
    pullUpDnControl(B3, PUD_UP);

    pinMode(TR1, INPUT);
    pullUpDnControl(TR1, PUD_UP);
    pinMode(TR2, INPUT);
    pullUpDnControl(TR2, PUD_UP);
    pinMode(TR3, INPUT);
    pullUpDnControl(TR3, PUD_UP);
    pinMode(TR4, INPUT);
    pullUpDnControl(TR4, PUD_UP);

    // LED + gate outputs (??) (what should those do?)
    pinMode(LED, OUTPUT);
    pinMode(GATE1, OUTPUT);
    pinMode(GATE2, OUTPUT);

    // interrupt, trigger inputs: 
    wiringPiISR (TR1, INT_EDGE_FALLING, &Interrupt_TR1) ;
    wiringPiISR (TR2, INT_EDGE_FALLING, &Interrupt_TR2) ;
    wiringPiISR (TR3, INT_EDGE_FALLING, &Interrupt_TR3) ;
    wiringPiISR (TR4, INT_EDGE_FALLING, &Interrupt_TR4) ;
    
    // interrupt, buttons: 
    wiringPiISR (B1, INT_EDGE_FALLING, &Interrupt_B1)  ;
    wiringPiISR (B2, INT_EDGE_FALLING, &Interrupt_B2)  ; 
    wiringPiISR (B3, INT_EDGE_FALLING, &Interrupt_B3)  ; 

    // flags
    int _wait = 0, _send = 0, toggle = 0;

    // main action is happening here (..)
     while(1)
    {

    	if (_wait < TIMEOUT) _wait++;
    	else {

            _wait = 0; // reset counter

            // get adc values

            for (int i = 0; i < ADC_NUM_CHANNELS; i++) {
    	         _send += readADC(i, adc);
            }
            // do we need to send an OSC bundle? 
            if (_send) {
              
                  // construct OSC bundle:
                  tosc_writeBundle(&bundle, NOW, buf, BUFLEN);
                  tosc_writeNextMessage(&bundle, "/adc0", "i", adc[0]);
                  tosc_writeNextMessage(&bundle, "/adc1", "i", adc[1]);
                  tosc_writeNextMessage(&bundle, "/adc2", "i", adc[2]);
                  tosc_writeNextMessage(&bundle, "/adc3", "i", adc[3]);
                  tosc_writeNextMessage(&bundle, "/adc4", "i", adc[4]);
                  tosc_writeNextMessage(&bundle, "/adc5", "i", adc[5]);
                  // ... and send bundle:
                  int len = tosc_getBundleLength(&bundle);

                  if (sendto(s, buf, len, 0 , (struct sockaddr *) &_serv, slen)==-1) die("sendto()");

            }
            // reset flag
            _send = 0;

            // handle buttons:
            if (B1_flag) { 

              B1_flag =  0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/button1", "T");
              // send message:
              if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) B1_OFF = 1;
              _wait_B1 = 0;
            }

            if (B2_flag) { 

              B2_flag =  0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/button2", "T");
              // send message:
              if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) B2_OFF = 1;
              _wait_B2 = 0;
            }

            // toggle illum. button 
            if (B3_flag) { 

              B3_flag =  0;
              toggle = ~toggle & 1u;
              int len = tosc_writeMessage(buf, sizeof(buf), "/button3", "i", toggle);
              // send message:
              sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
              digitalWrite(LED, toggle);
            }
    
            // turn off button1
            if (B1_OFF && _wait_B1 < TIMEOUT_S) _wait_B1++;
            else if (B1_OFF && _wait_B1 >= TIMEOUT_S){
              // reset
              B1_OFF = _wait_B1 = 0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/button1", "F");
              sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
            } 
            // turn off button2
            if (B2_OFF && _wait_B2 < TIMEOUT_S) _wait_B2++;
            else if (B2_OFF && _wait_B2 >= TIMEOUT_S) {
              // reset
              B2_OFF = _wait_B2 = 0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/button2", "F");
              sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
            } 
      }

      // handle trig inputs:

      if (TR1_flag) { 

              TR1_flag =  0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/trigger1", "T");
              // send message:
              if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) TR1_OFF = 1;
              _wait_TR1 = 0;
      }

      if (TR2_flag) { 

              TR2_flag =  0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/trigger2", "T");
              // send message:
              if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) TR2_OFF = 1;
              _wait_TR2 = 0;
      }

      if (TR3_flag) { 

              TR3_flag =  0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/trigger3", "T");
              // send message:
              if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) TR3_OFF = 1;
              _wait_TR3 = 0;
      }

      if (TR4_flag) { 

              TR4_flag =  0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/trigger4", "T");
              // send message:
              if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) TR4_OFF = 1;
              _wait_TR4 = 0;
      }

      // turn off gate1
      if (TR1_OFF && _wait_TR1 < TIMEOUT_T) _wait_TR1++;
      else if (TR1_OFF && _wait_TR1 >= TIMEOUT_T) {
              // reset
              TR1_OFF = _wait_TR1 = 0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/trigger1", "F");
              sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
      } 
      // turn off gate2
      if (TR2_OFF && _wait_TR2 < TIMEOUT_T) _wait_TR2++;
      else if (TR2_OFF && _wait_TR2 >= TIMEOUT_T) {
              // reset
              TR2_OFF = _wait_TR2 = 0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/trigger2", "F");
              sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
      } 
      // turn off gate3
      if (TR3_OFF && _wait_TR3 < TIMEOUT_T) _wait_TR3++;
      else if (TR3_OFF && _wait_TR3 >= TIMEOUT_T) {
              // reset
              TR3_OFF = _wait_TR3 = 0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/trigger3", "F");
              sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
      } 
      // turn off gate4
      if (TR4_OFF && _wait_TR4 < TIMEOUT_T) _wait_TR4++;
      else if (TR4_OFF && _wait_TR4 >= TIMEOUT_T) {
              // reset
              TR4_OFF = _wait_TR4 = 0;
              int len = tosc_writeMessage(buf, sizeof(buf), "/trigger4", "F");
              sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
      } 

            //printf("len .. %d \n", len);
            //clear the buffer
            //memset(buf,'\0', BUFLEN);
            //try to receive some data, this is a blocking call
     	   /*
    	   if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &_serve, &slen) == -1)
            {
                die("recvfrom()");
            }
            puts(buf);
    	   */
    }
    close(s);
    return 0;
}


