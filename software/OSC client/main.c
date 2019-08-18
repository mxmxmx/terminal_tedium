/*
*
*   terminal tedium / OSC client (UDP)
*
*     
*   - TD: gate outputs (?); make nicer. etc (pigpio?)
*   - TD: make more userfriendly, pass IP and port as arguments, etc.
*
*
*   compile with: gcc *.c -Werror -lwiringPi -std=gnu99 -O2 -g -o tedium_osc 
*/

#define DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>  

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "tinyosc.h"

const uint16_t BUFLEN = 1024;
const uint64_t NOW = 1L;
const uint32_t TIMEOUT = 1000;    // sleep (in us)
const uint32_t TIMEOUT_T = 10;    // short time out (for OSC off messages) (triggers) // 10 * TIMEOUT (us)

#define ADC_SPI_CHANNEL 1
#define ADC_SPI_SPEED 4000000
#define ADC_NUM_CHANNELS 6
#define ADC_NUM ADC_NUM_CHANNELS-1
#define RESOLUTION 4095
#define DEADBAND 2
#define SCALE 4000
#define SCALE_INV 1.0f/(float)SCALE

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

#define _MUTEX 0

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
      result = SCALE - (((spi_data[1] & 0x0f) << 8) | spi_data[2]);
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


///  buttons: 


enum button_states 
{
  OK,
  PRESSED,
  SHORT_PRESS,
  LONG_PRESS,
  SEND_OFF_MSG,
  SEND_OFF_MSG_L
};

typedef struct buttons
{
   uint8_t _pin;
   uint8_t _state;
   uint8_t _OSC_state;
   uint16_t _cnt;

} buttons;

buttons *button1, *button2, *button3;


void init_buttons() {

  button1 = (buttons*)malloc(sizeof(buttons));
  button2 = (buttons*)malloc(sizeof(buttons));
  button3 = (buttons*)malloc(sizeof(buttons));

  button1->_pin = B1;
  button1->_state = OK;
  button1->_OSC_state = OK;
  button1->_cnt = 0;

  button2->_pin = B2;
  button2->_state = OK;
  button2->_OSC_state = OK;
  button2->_cnt = 0;

  button3->_pin = B3;
  button3->_state = OK;
  button3->_OSC_state = OK;
  button3->_cnt = 0;

}

void check_buttons(struct buttons* _b, int time_out)
{

  if (_b->_state == PRESSED) {

        uint8_t _s = digitalRead(_b->_pin);
        if (!_s) _b->_cnt++;
        else if (_s && _b->_cnt < time_out)  _b->_state = SHORT_PRESS;
        else if (_s && _b->_cnt >= time_out) _b->_state = LONG_PRESS;

  }
  else if (_b->_state > PRESSED) { 

        piLock (_MUTEX);
          _b->_OSC_state = _b->_state;
        piUnlock (_MUTEX);  
        // reset
        _b->_state = OK;
        _b->_cnt = 0;
  }
}

///////////////////

PI_THREAD (handle_GPIO)
{

  uint16_t time_out = 125; // wait until longpress
  uint32_t _sleep = TIMEOUT*6;

  init_buttons();

  while(1) {

     usleep(_sleep);

     if (B1_flag && button1->_state == OK) { 

        B1_flag =  0;
        button1->_state = PRESSED;
     }

     if (B2_flag && button2->_state == OK) { 

        B2_flag =  0;
        button2->_state = PRESSED;
     }

     if (B3_flag && button3->_state == OK) { 

        B3_flag =  0;
        button3->_state = PRESSED;
     }

     check_buttons(button1, time_out);
     check_buttons(button2, time_out);
     check_buttons(button3, time_out);
  }
}

/* --------------------------------------------------------- */

int main(void)
{
    struct sockaddr_in _serv;
    int s, i, slen = sizeof(_serv);
    char buf[BUFLEN];
    int port = 9000;
    char *ip = "127.0.0.1"; 

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
    int _cnt = 0, _send = 0, toggle = 0;

    // thread to handle buttons
    piThreadCreate (handle_GPIO);

    // main action is happening here (..)
     while(1)
    {
            usleep(TIMEOUT);
            // get adc value
            _send += readADC(_cnt, adc);
            // do we need to send an OSC bundle? 
            if (_cnt == ADC_NUM && _send) {
              
                  // construct OSC bundle:
                  tosc_writeBundle(&bundle, NOW, buf, BUFLEN);
                  tosc_writeNextMessage(&bundle, "/adc0", "f", (float)adc[0] * SCALE_INV);
                  tosc_writeNextMessage(&bundle, "/adc1", "f", (float)adc[1] * SCALE_INV);
                  tosc_writeNextMessage(&bundle, "/adc2", "f", (float)adc[2] * SCALE_INV);
                  tosc_writeNextMessage(&bundle, "/adc3", "f", (float)adc[3] * SCALE_INV);
                  tosc_writeNextMessage(&bundle, "/adc4", "f", (float)adc[4] * SCALE_INV);
                  tosc_writeNextMessage(&bundle, "/adc5", "f", (float)adc[5] * SCALE_INV);
                  // ... and send bundle:
                  int len = tosc_getBundleLength(&bundle);

                  if (sendto(s, buf, len, 0 , (struct sockaddr *) &_serv, slen)==-1) die("sendto()");
                  // reset flag
                  _send = 0;
            }
            // increment adc cycle counter
            _cnt = _cnt++ >= ADC_NUM ? 0x0 : _cnt; // 0x5 = ADC_NUM_CHANNELS-1

            // buttons ?

            switch(button1->_OSC_state) {

              case OK:
                break;
              case SHORT_PRESS: {

                int len = tosc_writeMessage(buf, sizeof(buf), "/button1", "i", 1);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button1->_OSC_state = SEND_OFF_MSG;
                  piUnlock (_MUTEX);
                break;
              }

              case LONG_PRESS: {
                int len = tosc_writeMessage(buf, sizeof(buf), "/button1_long", "i", 1);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button1->_OSC_state = SEND_OFF_MSG_L;
                  piUnlock (_MUTEX);
                break;
              }
              case SEND_OFF_MSG: {
                int len = tosc_writeMessage(buf, sizeof(buf), "/button1", "i", 0);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button1->_OSC_state = OK;
                  piUnlock (_MUTEX);
                break;
              }
              case SEND_OFF_MSG_L: {
                int len = tosc_writeMessage(buf, sizeof(buf), "/button1_long", "i", 0);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);		
                  piLock (_MUTEX);
                    button1->_OSC_state = OK;
                  piUnlock (_MUTEX);
                break;
              }
              default: break;
          }  

          switch(button2->_OSC_state) {

              case OK:
                break;
              case SHORT_PRESS: {
                int len = tosc_writeMessage(buf, sizeof(buf), "/button2", "i", 1);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button2->_OSC_state = SEND_OFF_MSG;
                  piUnlock (_MUTEX);
                break;
              }
              case LONG_PRESS: {

                int len = tosc_writeMessage(buf, sizeof(buf), "/button2_long", "i", 1);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button2->_OSC_state = SEND_OFF_MSG_L;
                  piUnlock (_MUTEX);
                break;
              }
              case SEND_OFF_MSG: {

                int len = tosc_writeMessage(buf, sizeof(buf), "/button2", "i", 0);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button2->_OSC_state = OK;
                  piUnlock (_MUTEX);
                break;
              }
              case SEND_OFF_MSG_L: {

                int len = tosc_writeMessage(buf, sizeof(buf), "/button2_long", "i", 0);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button2->_OSC_state = OK;
                  piUnlock (_MUTEX);
                break;
              }
              default: break;
          } 

          switch(button3->_OSC_state) {

              case OK:
                break;
              case SHORT_PRESS: {
                int len = tosc_writeMessage(buf, sizeof(buf), "/button3", "i", 1);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button3->_OSC_state = SEND_OFF_MSG;
                  piUnlock (_MUTEX);
                  toggle = ~toggle & 1u;
                  digitalWrite(LED, toggle);
                break;
              }
              case LONG_PRESS: {

                int len = tosc_writeMessage(buf, sizeof(buf), "/button3_long", "i", 1);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button3->_OSC_state = SEND_OFF_MSG_L;
                  piUnlock (_MUTEX);
                  toggle = ~toggle & 1u;
                  digitalWrite(LED, toggle);
                break;
              }
              case SEND_OFF_MSG: {

                int len = tosc_writeMessage(buf, sizeof(buf), "/button3", "i", 0);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button3->_OSC_state = OK;
                  piUnlock (_MUTEX);
                break;
              }
              case SEND_OFF_MSG_L: {

                int len = tosc_writeMessage(buf, sizeof(buf), "/button3_long", "i", 0);
                  // send message:
                  sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
                  piLock (_MUTEX);
                    button3->_OSC_state = OK;
                  piUnlock (_MUTEX);
                break;
              }
              default: break;
          } 

          // handle trig inputs:

            if (TR1_flag && !TR1_OFF) { 

                    TR1_flag =  0;
                    int len = tosc_writeMessage(buf, sizeof(buf), "/trigger1", "i", 1);
                    // send message:
                    if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) TR1_OFF = 1;
                    _wait_TR1 = 0;
            }

            if (TR2_flag && !TR2_OFF) { 

                    TR2_flag =  0;
                    int len = tosc_writeMessage(buf, sizeof(buf), "/trigger2", "i", 1);
                    // send message:
                    if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) TR2_OFF = 1;
                    _wait_TR2 = 0;
            }

            if (TR3_flag && !TR3_OFF) { 

                    TR3_flag =  0;
                    int len = tosc_writeMessage(buf, sizeof(buf), "/trigger3", "i", 1);
                    // send message:
                    if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) TR3_OFF = 1;
                    _wait_TR3 = 0;
            }

            if (TR4_flag && !TR4_OFF) { 

                    TR4_flag =  0;
                    int len = tosc_writeMessage(buf, sizeof(buf), "/trigger4", "i", 1);
                    // send message:
                    if (sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen)) TR4_OFF = 1;
                    _wait_TR4 = 0;
            }

            // turn off gate1
            if (TR1_OFF && _wait_TR1 < TIMEOUT_T) _wait_TR1++;
            else if (TR1_OFF && _wait_TR1 >= TIMEOUT_T) {
                    // reset
                    TR1_OFF = _wait_TR1 = 0;
                    int len = tosc_writeMessage(buf, sizeof(buf), "/trigger1", "i", 0);
                    sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
            } 
            // turn off gate2
            if (TR2_OFF && _wait_TR2 < TIMEOUT_T) _wait_TR2++;
            else if (TR2_OFF && _wait_TR2 >= TIMEOUT_T) {
                    // reset
                    TR2_OFF = _wait_TR2 = 0;
                    int len = tosc_writeMessage(buf, sizeof(buf), "/trigger2", "i", 0);
                    sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
            } 
            // turn off gate3
            if (TR3_OFF && _wait_TR3 < TIMEOUT_T) _wait_TR3++;
            else if (TR3_OFF && _wait_TR3 >= TIMEOUT_T) {
                    // reset
                    TR3_OFF = _wait_TR3 = 0;
                    int len = tosc_writeMessage(buf, sizeof(buf), "/trigger3", "i", 0);
                    sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
            } 
            // turn off gate4
            if (TR4_OFF && _wait_TR4 < TIMEOUT_T) _wait_TR4++;
            else if (TR4_OFF && _wait_TR4 >= TIMEOUT_T) {
                    // reset
                    TR4_OFF = _wait_TR4 = 0;
                    int len = tosc_writeMessage(buf, sizeof(buf), "/trigger4", "i", 0);
                    sendto(s, buf, len, 0, (struct sockaddr *) &_serv, slen);
            }
    }
    close(s);
    return 0;
}
// end