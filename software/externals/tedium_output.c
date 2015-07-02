/***********************************************************************
 * terminal tedium: GPIO
 * out (inlets 2, 3) : GPIO 16, 26 [odroid w = 34, 35]
 * ****************************************************************************/

#include "m_pd.h"
#include <stdint.h>
#ifdef __arm__
    #include <wiringPi.h>
#endif


/* a+, b+ : GPIO16, 26; if using odroid w, change pins to GPIO34 and 35 */


const uint8_t CLK1 = 16; // 34
const uint8_t CLK2 = 26; // 35

static t_class *tedium_output_class;

typedef struct _tedium_output
{
    t_object x_obj;
    t_inlet  *x_in1; 
    t_inlet  *x_in2; 
    t_float state16;
    t_float state26;
} t_tedium_output;


static void tedium_output_pin16(t_tedium_output *x, t_floatarg msg)
{
    if (msg > 0)     x->state16 = 1; //digitalWrite(16, HIGH); // pull high
    else             x->state16 = 0; //digitalWrite(16, LOW);  // pull low
    #ifdef __arm__
        digitalWrite(CLK1, x->state16);
    #endif

}


static void tedium_output_pin26(t_tedium_output *x, t_floatarg msg)
{
    if (msg > 0)     x->state26 = 1; //digitalWrite(26, HIGH);// pull high
    else             x->state26 = 0; //digitalWrite(26, LOW); // pull low
    #ifdef __arm__
        digitalWrite(CLK2, x->state26);
    #endif
}

/*
static void tedium_output_bang(t_tedium_output *x)
{
// do nothing
}
*/


void *tedium_output_new(t_floatarg _state16, t_floatarg _state26){

    t_tedium_output *x = (t_tedium_output *)pd_new(tedium_output_class);

    x->x_in1 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("pin26"));
    x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("pin16"));

    #ifdef __arm__
        pinMode(CLK1, OUTPUT);
        pinMode(CLK2, OUTPUT);
    #endif

    x->state16 = _state16;
    x->state26 = _state26;
    #ifdef __arm__
         digitalWrite(CLK1, x->state16);
         digitalWrite(CLK2, x->state26);
    #endif    
    return (void *)x;
}


/* -------------------------------------- */

void tedium_output_setup(void)
{
    #ifdef __arm__
        wiringPiSetupGpio();
    #endif
    tedium_output_class = class_new(gensym("tedium_output"),
		    (t_newmethod)tedium_output_new,
                    0, sizeof(t_tedium_output),
		    CLASS_DEFAULT,
		    A_DEFFLOAT, A_DEFFLOAT,
                    0);
    //class_addbang(tedium_output_class,  tedium_output_bang);  // ?
    class_addmethod(tedium_output_class, (t_method)tedium_output_pin16, gensym("pin16"), A_FLOAT, 0);
    class_addmethod(tedium_output_class, (t_method)tedium_output_pin26, gensym("pin26"), A_FLOAT, 0);
}



