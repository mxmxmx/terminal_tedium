#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <netinet/tcp.h>
#include <unistd.h>     
#include <sys/time.h>

#include <math.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>


#define ADC_SPI_CHANNEL 1
#define ADC_SPI_SPEED 1000000
#define ADC_NUM_CHANNELS 8
#define RESOLUTION 4095 // 1023 if using MCP3008; 4095 if using MCP3208
#define DEADBAND 2

void die(char *errorMessage)
{
  perror(errorMessage);
  exit(1);
}

// interrupt things

/*
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


// FUDI messages for triggers and buttons 
const char FUDI1[5] = {'8', '\t', '1', ';', '\0'};
const char FUDI2[5] = {'9', '\t', '1', ';', '\0'};
const char FUDI3[6] = {'1', '0', '\t', '1', ';', '\0'};
const char FUDI4[6] = {'1', '1', '\t', '1', ';', '\0'};
const char FUDI5[6] = {'1', '2', '\t', '1', ';', '\0'};
const char FUDI6[6] = {'1', '3', '\t', '1', ';', '\0'};
const char FUDI7[6] = {'1', '4', '\t', '1', ';', '\0'};
*/

uint8_t SENDMSG;

// ADC :

uint16_t adc[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //  store prev.
uint8_t  map_adc[8] = {5, 2, 7, 6, 3, 0, 1, 4}; // map to panel [1 - 2 - 3; 4 - 5 - 6; 7, 8]

uint16_t readADC(int _channel){ // 12 bit

	uint8_t spi_data[3];
	uint8_t input_mode = 1; // single ended = 1, differential = 0
	uint16_t result, tmp;

	spi_data[0] = 0x04; // start flag
	spi_data[0] |= (input_mode<<1); // shift input_mode
	spi_data[0] |= (_channel>>2) & 0x01; // add msb of channel in our first command byte

	spi_data[1] = _channel<<6;
	spi_data[2] = 0x00;

	wiringPiSPIDataRW(ADC_SPI_CHANNEL, spi_data, 3);
	result = (spi_data[1] & 0x0f)<<8 | spi_data[2];
        tmp = adc[_channel]; // prev.
	if ( (result - tmp) > DEADBAND || (tmp - result) > DEADBAND ) { tmp = result ; SENDMSG = 1; }
        adc[_channel] = tmp;
	return tmp;
}

uint16_t readADC_10bit(int _channel){ // if using mcp3008

	uint8_t spi_data[3];
	uint16_t result, tmp;

	spi_data[0] = 0x01; // start flag
	spi_data[1] = (_channel + 8) << 4;
	spi_data[2] = 0x00;

	wiringPiSPIDataRW(ADC_SPI_CHANNEL, spi_data, 3);

	result = (spi_data[1] & 3)<<8 | spi_data[2];
        tmp = adc[_channel]; // prev.
        if ( (result - tmp) > DEADBAND || (tmp - result) > DEADBAND ) tmp = result ;
        adc[_channel] = tmp;
	return tmp;
}


int main(int argc, char *argv[]){
      
	int sock;                        /* Socket descriptor */
    	struct sockaddr_in echoServAddr; /* Echo server address */
    	unsigned short echoServPort;     /* Echo server port */
    	char *servIP;                    /* Server IP address (dotted quad) */
    	int rate;                        /* rate (time between each send) */
     	int value = 1;
 
	uint32_t adc_counter  = 0;
	uint8_t  adc_counter2 = 0;

    	if (argc != 4)    /*  correct number of arguments ? */
    	{
      		 fprintf(stderr, "Usage: %s <Server IP> <Server Port> <rate>\n", argv[0]);
       		 exit(1);
    	}

   	servIP = argv[1];              /* server IP address */
   	echoServPort = atoi(argv[2]);  /* server port */
    	rate = atoi(argv[3]);          /* time between each send (in milliseconds (not quite)) */

	rate *= 3000; /*  counter threshold */

    	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        	die("socket() failed");

    	memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    	echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    	echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    	echoServAddr.sin_port        = htons(echoServPort); /* Server port */
	
	
    	/* connection to the echo server */
    	if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        die("connect() failed");
	

    	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof(int)) < 0)
      	die("TCP_NODELAY failed");

    	
	wiringPiSetupGpio();
	wiringPiSPISetup(ADC_SPI_CHANNEL, ADC_SPI_SPEED);

   /*
	wiringPiISR (4,  INT_EDGE_FALLING, &Interrupt_TR1) ;
  	wiringPiISR (17, INT_EDGE_FALLING, &Interrupt_TR2) ;
  	wiringPiISR (2,  INT_EDGE_FALLING, &Interrupt_TR3) ;
  	wiringPiISR (3,  INT_EDGE_FALLING, &Interrupt_TR4) ;
  	wiringPiISR (23, INT_EDGE_FALLING, &Interrupt_B1)  ;
	wiringPiISR (25, INT_EDGE_FALLING, &Interrupt_B2)  ; 
	wiringPiISR (24, INT_EDGE_FALLING, &Interrupt_B3)  ; 
   */	

	for(;;){

		// digital inputs: interrupt?
       /*
		if (TR1_flag) { 
			TR1_flag = 0;
			send(sock, FUDI1, 4, 0); // strlen(FUDI2) = 4
		}
		if (TR2_flag) { 
			TR2_flag = 0;	
			send(sock, FUDI2, 4, 0); // strlen(FUDI2) = 4
		}
		if (TR3_flag) { 
			TR3_flag = 0;
			send(sock, FUDI3, 5, 0); // strlen(FUDI3) = 5
		}
		if (TR4_flag) { 
			TR4_flag = 0;
			send(sock, FUDI4, 5, 0); // strlen(FUDI4) = 5
		}
		if ( B1_flag) { 
			B1_flag =  0;
			send(sock, FUDI5, 5, 0); // strlen(FUDI5) = 5
		}
		if ( B2_flag) {  
			B2_flag =  0;
			send(sock, FUDI6, 5, 0); // strlen(FUDI6) = 5
		}
		if ( B3_flag) {  
			B3_flag =  0;
			send(sock, FUDI7, 5, 0); // strlen(FUDI7) = 5
		}
        */
                /// ADC: 
	     
		adc_counter++;
				
		if (adc_counter > rate){
                        
			adc_counter = 0; // reset counter
			adc_counter2++;  // incr  counter #2
			if (adc_counter2 >= ADC_NUM_CHANNELS) adc_counter2 = 0; 
                        uint8_t *adc_ptr = map_adc;
			adc_ptr += adc_counter2; // -> map to panel
			int16_t val = 0;
			uint8_t msgLength;
			char *ADC_FUDI;

			val = RESOLUTION - readADC(adc_counter2);
			
			if (SENDMSG) {
				if (val > 999)      msgLength = 8; 
				else if (val > 99)  msgLength = 7; 
				else if (val > 9)   msgLength = 6; 
				else 		      	msgLength = 5; 
				// format FUDI msg: { id, whitespace, val, semicolon, /0 }
				ADC_FUDI = (char *) malloc(msgLength); 
				snprintf(ADC_FUDI+2, msgLength, "%d", val);
				ADC_FUDI[0] = (char)(0x30+*adc_ptr); // + ascii offset
				ADC_FUDI[1] = ' ';
				ADC_FUDI[msgLength-2] = ';';
			
				send(sock, ADC_FUDI, strlen(ADC_FUDI), 0); 
			}
			SENDMSG = 0;
			// if (i == 5) { printf("%d \n", adc[i]);} 				
		}

	
       		
	}
}
