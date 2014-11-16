/***********************************************************************
 * terminal tedium: GPIO
 * out (inlets) : GPIO 16, 26
 * in  (outlets): GPIO 4, 17, 2, 3, 23, 25, 24  
 * ****************************************************************************/

#include "m_pd.h"
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>


/* -------------------------------------- */


static t_class *tt_gpio_class;

typedef struct _tt_gpio
{
    t_object x_obj;
    t_inlet  *x_in1; // clock 16 [0 or 1]
    t_inlet  *x_in2; // clock 26 [0 or 1]
    t_outlet *x_out1;
    t_outlet *x_out2;
    t_outlet *x_out3;
    t_outlet *x_out4;
    t_outlet *x_out5;
    t_outlet *x_out6;
    t_outlet *x_out7;

    t_float state16;
    t_float state26;
   
} t_tt_gpio;


// ? TR flag etc— volatile?
volatile int TR1_flag;    
volatile int TR2_flag;
volatile int TR3_flag;
volatile int TR4_flag;
volatile int B1_flag;
volatile int B2_flag;
volatile int B3_flag;


/* -------------------------------------- */


static void tt_gpio_pin16(t_tt_gpio *x, t_floatarg msg)
{
    if (msg > 0)     x->state16 = 1; //digitalWrite(16, HIGH); // pull high
    else             x->state16 = 0; //digitalWrite(16, LOW);  // pull low
    digitalWrite(16, x->state16);	

}


static void tt_gpio_pin26(t_tt_gpio *x, t_floatarg msg)
{
    if (msg > 0)     x->state26 = 1; //digitalWrite(26, HIGH);// pull high
    else             x->state26 = 0; //digitalWrite(26, LOW); // pull low
    digitalWrite(26, x->state26);
}


static void tt_gpio_bang(t_tt_gpio *x) 
{
   
    if (TR1_flag) { 
	TR1_flag = 0;
        outlet_bang(x->x_out1);
    }
    if (TR2_flag) { 
	TR2_flag = 0;
        outlet_bang(x->x_out2);
    }
    if (TR3_flag) { 
	TR3_flag = 0;
        outlet_bang(x->x_out3);
    }	
    if (TR4_flag) { 
	TR4_flag = 0;
        outlet_bang(x->x_out4);
    }
    if (B1_flag) { 
	B1_flag = 0;
        outlet_bang(x->x_out5);
    }
    if (B2_flag) { 
	B2_flag = 0;
        outlet_bang(x->x_out6);
    }		
    if (B3_flag) { 
	B3_flag = 0;
        outlet_bang(x->x_out7);
    }			
	
}


static void Interrupt_TR1 () { TR1_flag = 1; }
static void Interrupt_TR2 () { TR2_flag = 1; }
static void Interrupt_TR3 () { TR3_flag = 1; }
static void Interrupt_TR4 () { TR4_flag = 1; }
static void Interrupt_B1  () { B1_flag  = 1; }
static void Interrupt_B2  () { B2_flag  = 1; }
static void Interrupt_B3  () { B3_flag  = 1; }


/* -------------------------------------- */

static t_tt_gpio tt_gpio_new(t_floatarg _state16, t_floatarg _state26){

    t_tt_gpio *x = (t_tt_gpio *)pd_new(tt_gpio_class);

    x->x_in1 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("pin26"));
    x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("pin16")); 
    
    x->x_out1 = outlet_new(&x->x_obj, &s_bang);
    x->x_out2 = outlet_new(&x->x_obj, &s_bang);
    x->x_out3 = outlet_new(&x->x_obj, &s_bang);
    x->x_out4 = outlet_new(&x->x_obj, &s_bang);
    x->x_out5 = outlet_new(&x->x_obj, &s_bang);
    x->x_out6 = outlet_new(&x->x_obj, &s_bang);
    x->x_out7 = outlet_new(&x->x_obj, &s_bang);
	
    if (wiringPiSetupGpio() == -1)	exit (1);

    pinMode(16, OUTPUT);
    pinMode(26, OUTPUT);

    pinMode(4,  INPUT);
    pinMode(17, INPUT);
    pinMode(2,  INPUT);
    pinMode(3,  INPUT);
    pinMode(23, INPUT);
    pinMode(25, INPUT); 
    pinMode(24, INPUT);
			
    // pull ups? only buttons		
    // pullUpDnControl(4,  PUD_UP);
    // pullUpDnControl(17, PUD_UP);
    // pullUpDnControl(2,  PUD_UP);
    // pullUpDnControl(3,  PUD_UP);	
    pullUpDnControl(23, PUD_UP);
    pullUpDnControl(25, PUD_UP);
    pullUpDnControl(24, PUD_UP);

    wiringPiISR (4,  INT_EDGE_FALLING, &Interrupt_TR1) ;
    wiringPiISR (17, INT_EDGE_FALLING, &Interrupt_TR2) ;
    wiringPiISR (2,  INT_EDGE_FALLING, &Interrupt_TR3) ;
    wiringPiISR (3,  INT_EDGE_FALLING, &Interrupt_TR4) ;
    wiringPiISR (23, INT_EDGE_FALLING, &Interrupt_B1)  ;
    wiringPiISR (25, INT_EDGE_FALLING, &Interrupt_B2)  ; 
    wiringPiISR (24, INT_EDGE_FALLING, &Interrupt_B3)  ;
	
    x->state16 = _state16;
    x->state26 = _state26;

    x->TR1_flag = 0;
    x->TR2_flag = 0;
    x->TR3_flag = 0;
    x->TR4_flag = 0;
    x->B1_flag  = 0;
    x->B2_flag  = 0;
    x->B3_flag  = 0;

    digitalWrite(16, x->state16);    
    digitalWrite(26, x->state26); 
    
    return (void *)x;
}


/* -------------------------------------- */

void tt_gpio_setup(void)
{
    tt_gpio_class = class_new(gensym("tt_gpio"), 
		    (t_newmethod)tt_gpio_new,
                    0, sizeof(t_tt_gpio), 
		    CLASS_DEFAULT,
		    A_DEFFLOAT, A_DEFFLOAT,	
                    0); 
    class_addbang(tt_gpio_class,  tt_gpio_bang); // this is to scan the inputs ?
    class_addmethod(tt_gpio_class, (t_method)tt_gpio_pin16, gensym("pin16"), A_FLOAT, 0);
    class_addmethod(tt_gpio_class, (t_method)tt_gpio_pin26, gensym("pin26"), A_FLOAT, 0); 
  
		
}



