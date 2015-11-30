/***********************************************************************
 * terminal tedium: GPIO
 * outputs: 
 * pcm5102a version: GPIO 16, 26 
 * wm8731   version: GPIO 12, 16
 * ****************************************************************************/


#include "m_pd.h"
#include <stdio.h>
#ifdef __arm__
	#include <wiringPi.h>
#endif

t_class *tedium_output_class;

typedef struct _tedium_output
{
	t_object x_obj;
	t_int clkState;
	t_int pinNum;

} t_tedium_output;

void tedium_output_gate(t_tedium_output *x, t_floatarg _gate)
{
	if (_gate > 0)	x->clkState = 1; 
	else		x->clkState = 0;
	#ifdef __arm__
		digitalWrite(x->pinNum, x->clkState);
	#endif
}

void *tedium_output_new(t_floatarg _pin)
{
	t_tedium_output *x = (t_tedium_output *)pd_new(tedium_output_class);

	// valid pin?
	if (_pin == 12 || _pin == 16 || _pin == 26) x->pinNum = _pin;
	else x->pinNum = 16; // default to pin #16
	#ifdef __arm__
		pinMode(x->pinNum, OUTPUT);
	#endif
	x->clkState = 0;
	return (void *)x;
}



void tedium_output_setup()
{
	#ifdef __arm__
		wiringPiSetupGpio();
	#endif	
	tedium_output_class = class_new(gensym("tedium_output"),
		(t_newmethod)tedium_output_new,
		0, sizeof(t_tedium_output), 
		CLASS_DEFAULT, 
		A_DEFFLOAT,
		0);
	class_addfloat(tedium_output_class, (t_method)tedium_output_gate);
}
