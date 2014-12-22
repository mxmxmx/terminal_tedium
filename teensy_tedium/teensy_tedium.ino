/*
 *
 * teensy adapter test sketch
 *
 *
 * ... wherein not a great deal happens: 
 *
 * - sine wave(s) should be present at the audio outputs (different frequencies at L / R);
 * - clocking any of the digital inputs should make the frequency change, ie play a sequence; 
 * - clocking them will also toggle the digital outputs / LEDs;
 * - the adc values are printed to the serial monitor; 
 * - pressing the buttons should print something to the monitor, too.
 *
 *
 */
 
#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>


/* pin usage */

#define GPIO3 0
#define GPIO4 1
#define GPIO17 2
#define GPIO23 3
#define GPIO24 4
#define GPIO16 5
#define MEMCS 6
// MOSI 7
#define GPIO25 8
// BCLK 9
#define ADC_CS 10
// unused 11
// MISO 12
// unused 13
// SPI_CLK 14
#define GPIO26 15
// unused 16
// unused 17
// unused 18
// unused 19
#define GPIO2 20
// unused 21
// DOUT 22
// LRCK 23


// some audio lib stuff for testing 
AudioSynthWaveform sine0, sine1, sine2, sine3;

AudioSynthWaveform *waves[4] = {
  &sine0, &sine1, &sine2, &sine3
};

AudioMixer4     mixL;
AudioMixer4     mixR;

AudioOutputI2S  PCM5102;

AudioConnection c0(sine0, 0, mixL, 0);
AudioConnection c1(sine1, 0, mixL, 1);
AudioConnection c2(sine2, 0, mixR, 0);
AudioConnection c3(sine3, 0, mixR, 1);

AudioConnection c4(mixL, 0, PCM5102, 0);
AudioConnection c5(mixR, 0, PCM5102, 1);


/* ADC stuff */

#define numADC 8
SPISettings MCP3208(200000, MSBFIRST, SPI_MODE0); 

#define ADC_rate 5000
IntervalTimer ADC_timer;
volatile boolean _ADC = false;
void ADC_callback() { _ADC = true; }
uint8_t ADC_counter; 
uint16_t adc_val[numADC]; // store 
const uint8_t  map_adc[numADC] = {5, 4, 6, 7, 1, 0, 2, 3}; // map channels to panel


/* digital inputs / ISRs  */

volatile boolean CLK_STATE1;
void clk_ISR1() {  CLK_STATE1 = true; }

volatile boolean CLK_STATE2;
void clk_ISR2() {  CLK_STATE2 = true; }

volatile boolean CLK_STATE3;
void clk_ISR3() {  CLK_STATE3 = true; }

volatile boolean CLK_STATE4;
void clk_ISR4() {  CLK_STATE4 = true; }

/* timer etc for checking the buttons */
#define UI_rate 50000
IntervalTimer UI_timer;
volatile boolean _UI = false;
void UI_callback() { _UI = true; }

uint8_t DEBOUNCE = 250; 
uint32_t LAST_UI;


void setup()
{

  AudioMemory(18);
  
  // chip select 
  pinMode(ADC_CS, OUTPUT);
  
  // clock outputs
  pinMode(GPIO26, OUTPUT);
  pinMode(GPIO16, OUTPUT);
  
  // clock inputs
  pinMode(GPIO4, INPUT);
  pinMode(GPIO17, INPUT);
  pinMode(GPIO2, INPUT);
  pinMode(GPIO3, INPUT);
  
  // buttons (need pullups)
  pinMode(GPIO24, INPUT_PULLUP);
  pinMode(GPIO25, INPUT_PULLUP);
  pinMode(GPIO23, INPUT_PULLUP);
  
  SPI.setMOSI(7);  // MOSI on alt/pin 7
  SPI.setSCK(14);  // SCK  on alt/pin 14

  sine0.begin(0.2,440,WAVEFORM_SINE);
  sine1.begin(0.2,220,WAVEFORM_SINE);
  sine2.begin(0.2,120,WAVEFORM_SINE);
  sine3.begin(0.2,120,WAVEFORM_SINE);
  
  delay(100);
  /* ADC timer */
  ADC_timer.begin(ADC_callback, ADC_rate);
  /* UI timer */
  UI_timer.begin(UI_callback, UI_rate);
  /* SPI */
  SPI.begin(); 
  /* clock inputs  */
  attachInterrupt(GPIO4,  clk_ISR1, FALLING);
  attachInterrupt(GPIO17, clk_ISR2, FALLING);
  attachInterrupt(GPIO2,  clk_ISR3, FALLING);
  attachInterrupt(GPIO3,  clk_ISR4, FALLING);
  /* LEDs on */
  digitalWriteFast(GPIO26, HIGH);
  digitalWriteFast(GPIO16, HIGH);
}

/* a few more variables needed below */

uint8_t cnt, LEN = 6, xxx;
uint16_t freq;
uint16_t SEQ[] = {440, 880, 660, 880, 660, 330};


void loop()
{
  
   /* read the channels + print the values, every now and then*/
   if (_ADC) { 
      _ADC = false;
      read_MCP3208();
    }   
  
   /* check digital inputs 1-4*/
   
   if (CLK_STATE1)   {
       CLK_STATE1 = false;
       dosomething();    
   }  
   
   
   if (CLK_STATE2)   {
       CLK_STATE2 = false;
       dosomething();    
   }  
   
   
   if (CLK_STATE3)   {
       CLK_STATE3 = false;
       dosomething();    
   }  
   
   
   if (CLK_STATE4)   {
       CLK_STATE4 = false;
       dosomething();    
   }  
   
   /* buttons pressed ? */
   if (_UI) { 
     
       _UI = false;
       if (!digitalReadFast(GPIO23) && millis() - LAST_UI > DEBOUNCE)  { Serial.println("B 1"); LAST_UI = millis(); }
       if (!digitalReadFast(GPIO24) && millis() - LAST_UI > DEBOUNCE)  { Serial.println("B 3"); LAST_UI = millis(); }
       if (!digitalReadFast(GPIO25) && millis() - LAST_UI > DEBOUNCE)  { Serial.println("B 2"); LAST_UI = millis(); }
   }
}



/* audio +  output test */ 

void dosomething() {
  
       cnt++;
       if (cnt >=LEN) cnt = 0;
       freq = SEQ[cnt];
       
       AudioNoInterrupts();
       waves[0]->frequency(freq);
       waves[1]->frequency(freq/3);
       waves[2]->frequency(freq/4);
       waves[3]->frequency(freq*1.5);
       AudioInterrupts();
       /* toggle digital outputs */
       digitalWriteFast(GPIO26, xxx);
       xxx = ~xxx & 1u;
       digitalWriteFast(GPIO16, xxx); 
}  


/* this is only for convenience, see below for the SPI transfer 
   in real life, you'll probably want to do some averaging / de-jittering / etc
*/

void read_MCP3208() {
   
      ADC_counter++;
    
      if (ADC_counter < numADC) {
           uint8_t n = ADC_counter;
           adc_val[n] = readADC(map_adc[n]); 
      }
      else {
           uint8_t n = ADC_counter = 0;
           adc_val[n] = readADC(map_adc[n]);
           
           for (int i = 0; i < numADC; i++) {
                 Serial.print(adc_val[i]);
                 Serial.print(" | ");
           }
           Serial.println(" ");  
      }    
}
  
/* read ADC */

uint16_t readADC(int _channel){ 

        SPI.beginTransaction(MCP3208); 

        uint8_t  commandMSB = B00000110;
        uint16_t commandBytes = (uint16_t) ((commandMSB<<8) | (_channel<<6));
 
        digitalWriteFast(ADC_CS, LOW);
        
        SPI.transfer((commandBytes>>8) & 0xff);
        byte _msb = SPI.transfer((byte)commandBytes & 0xff) & B00001111;
        byte _lsb = SPI.transfer(0x00);
        
        digitalWriteFast(ADC_CS,HIGH);
      
        return 4096 - (((uint16_t) _msb) << 8 | _lsb);
}
