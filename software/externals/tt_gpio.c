/***********************************************************************
 * terminal tedium: GPIO
 * out (inlets 2, 3) : GPIO 16, 26 [odroid w = 34, 35]
 * ****************************************************************************/

#include "m_pd.h"
#include <stdint.h>
#include <wiringPi.h>


/* a+, b+ : GPIO16, 26; if using odroid w, change pins to GPIO34 and 35 */


const uint8_t CLK1 = 16; // 34
const uint8_t CLK2 = 26; // 35

static t_class *tt_gpio_class;

typedef struct _tt_gpio
{
    t_object x_obj;
    t_inlet  *x_in1; 
    t_inlet  *x_in2; 
    t_float state16;
    t_float state26;
} t_tt_gpio;


static void tt_gpio_pin16(t_tt_gpio *x, t_floatarg msg)
{
    if (msg > 0)     x->state16 = 1; //digitalWrite(16, HIGH); // pull high
    else             x->state16 = 0; //digitalWrite(16, LOW);  // pull low
    digitalWrite(CLK1, x->state16);

}


static void tt_gpio_pin26(t_tt_gpio *x, t_floatarg msg)
{
    if (msg > 0)     x->state26 = 1; //digitalWrite(26, HIGH);// pull high
    else             x->state26 = 0; //digitalWrite(26, LOW); // pull low
    digitalWrite(CLK2, x->state26);
}


static void tt_gpio_bang(t_tt_gpio *x)
{
// do nothing
}


void *tt_gpio_new(t_floatarg _state16, t_floatarg _state26){

    t_tt_gpio *x = (t_tt_gpio *)pd_new(tt_gpio_class);

    x->x_in1 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("pin26"));
    x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("pin16"));

    pinMode(CLK1, OUTPUT);
    pinMode(CLK2, OUTPUT);

    x->state16 = _state16;
    x->state26 = _state26;

    digitalWrite(CLK1, x->state16);
    digitalWrite(CLK2, x->state26);
    return (void *)x;
}


/* -------------------------------------- */

void tt_gpio_setup(void)
{

   wiringPiSetupGpio();
   tt_gpio_class = class_new(gensym("tt_gpio"),
		    (t_newmethod)tt_gpio_new,
                    0, sizeof(t_tt_gpio),
		    CLASS_DEFAULT,
		    A_DEFFLOAT, A_DEFFLOAT,
                    0);
    class_addbang(tt_gpio_class,  tt_gpio_bang);  // ?
    class_addmethod(tt_gpio_class, (t_method)tt_gpio_pin16, gensym("pin16"), A_FLOAT, 0);
    class_addmethod(tt_gpio_class, (t_method)tt_gpio_pin26, gensym("pin26"), A_FLOAT, 0);
}



