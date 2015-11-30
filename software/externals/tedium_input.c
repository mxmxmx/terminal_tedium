/* NB: pullups need to be set for the inputs to work */


#include "m_pd.h"
#include <stdio.h>
#ifdef __arm__
	#include <wiringPi.h>
#endif

t_class *tedium_input_class;

typedef struct _tedium_input
{
	t_object x_obj;
	t_clock *x_clock;
	t_int clkState;
	t_int pinNum;
	t_outlet *x_out;

} t_tedium_input;

void tedium_input_tick(t_tedium_input *x)
{
	int prevState = x->clkState;
	#ifdef __arm__
		x->clkState = digitalRead(x->pinNum); 
	#endif
	// pin pulled low since last tick ?
	if(prevState && !x->clkState) outlet_bang(x->x_out);
	clock_delay(x->x_clock, 0x1); 
}

void *tedium_input_new(t_floatarg _pin)
{
	t_tedium_input *x = (t_tedium_input *)pd_new(tedium_input_class);
	x->x_clock = clock_new(x, (t_method)tedium_input_tick);
	// valid pin?
	if (_pin == 4 || _pin == 17 || _pin == 2 || _pin == 3 || _pin == 14 || _pin == 27 || _pin == 23 || _pin == 24 || _pin == 25) x->pinNum = _pin;
	else x->pinNum = 4; // default to pin #4	
	#ifdef __arm__
		pinMode(x->pinNum, INPUT);
		pullUpDnControl(x->pinNum, PUD_UP);
	#endif
	x->x_out = outlet_new(&x->x_obj, gensym("bang"));
	tedium_input_tick(x);
	return (void *)x;
}

void tedium_input_free(t_tedium_input *x)
{
	clock_free(x->x_clock);
	outlet_free(x->x_out);  
}

void tedium_input_setup()
{
	#ifdef __arm__
		wiringPiSetupGpio();
	#endif	
	tedium_input_class = class_new(gensym("tedium_input"),
		(t_newmethod)tedium_input_new, (t_method)tedium_input_free,
		sizeof(t_tedium_input), 
		CLASS_NOINLET, 
		A_DEFFLOAT,
		0);
}
