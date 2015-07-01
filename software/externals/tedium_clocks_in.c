/* NB: pullups need to be set externally */


#include "m_pd.h"
#include <wiringPi.h>
#include <stdio.h>

t_class *tedium_clk_in_class;

typedef struct _tedium_clk_in
{
	t_object x_obj;
	t_clock *x_clock;
	t_int clkState;
	t_int pinNum;

} t_tedium_clk_in;

void tedium_clk_in_tick(t_tedium_clk_in *x)
{
	int prevState = x->clkState;
	x->clkState = digitalRead(x->pinNum); 
	 // pin pulled low since last tick ? 
    	if(prevState && !x->clkState) outlet_bang(x->x_obj.ob_outlet);
	clock_delay(x->x_clock, 0x1); 
}

void *tedium_clk_in_new(t_floatarg _pin)
{
	t_tedium_clk_in *x = (t_tedium_clk_in *)pd_new(tedium_clk_in_class);
	x->x_clock = clock_new(x, (t_method)tedium_clk_in_tick);
    	// valid pin? 
	// fprintf(stderr,"pin <%s>\n", _pin); 
        if (_pin == 4 || _pin == 17 || _pin == 2 || _pin == 3 || _pin == 23 || _pin == 24 || _pin == 25) x->pinNum = _pin;
        else x->pinNum = 4; // default to pin #4	
	pinMode(x->pinNum, INPUT);

    	outlet_new(&x->x_obj, gensym("bang"));
	tedium_clk_in_tick(x);
	return (void *)x;
}

void tedium_clk_in_free(t_tedium_clk_in *x)
{
	clock_free(x->x_clock);
}

void tedium_clk_in_setup()
{
	wiringPiSetupGpio();
	tedium_clk_in_class = class_new(gensym("tedium_clk_in"),
		(t_newmethod)tedium_clk_in_new, 
		0, sizeof(t_tedium_clk_in), 
		CLASS_NOINLET, 
		A_DEFFLOAT,
		0);
}
