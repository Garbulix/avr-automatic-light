#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define DDR_PIR DDRC
#define PORT_PIR PINC
#define INPUT_PIR PINC4

#define DDR_CONTROL DDRD
#define PORT_CONTROL PORTD
#define OUTPUT_CONTROL PORTD6

#define DDR_STATUS_LED DDRB
#define PORT_STATUS_LED PORTB
#define OUTPUT_STATUS_LED PORTB4

#define INPUT_PHOTO PORTC5

#define PWM_DUTY_REGISTER OCR0A

#define PWM_DELAY 5
#define DEFAULT_DELAY 500
#define LONG_DELAY 10000
#define MAX_DUTY 255

#define TRUE 1
#define FALSE 0


/********** CONSTANTS **********/
const uint16_t ADC_LIGHT_THRESHOLD = 250;	//when to assume there is dark
/********** CONSTANTS **********/



/********** GLOBAL VARIABLES **********/
uint8_t light_turned_on = FALSE;	//info about current light status
volatile uint8_t ADC_done = FALSE;	//info from ADC if the conversion form analog to digital is done
volatile uint8_t is_dark = FALSE;	//info about darkness (if it is dark or not)
uint8_t current_duty = 0;			//current PWM duty
/********** GLOBAL VARIABLES **********/



/********** FUNCTION DECLARATIONS **********/
void setup();						
void ADC_start_conversion();		
void activate_light(uint8_t fast);	//turn the lights on (can be done immidiately)
void deactivate_light_maybe();		//turning light off with constant checking about someones move
void set_duty(uint8_t duty);
/********** FUNCTION DECLARATIONS **********/


//interrupt from ADC with info about light level
ISR(ADC_vect) {
	if (ADC < ADC_LIGHT_THRESHOLD) {
		//light level below threshold -> it is dark  
		is_dark = TRUE;
	} else {
		is_dark = FALSE;
	}
	//info for another functions that conversion is done
	ADC_done = TRUE;
}


/**************************/
/********** MAIN **********/
/**************************/
int main() {
	setup();	
	sei();		

	while (1) {
		if (light_turned_on == FALSE) {	
			if ( PORT_PIR & _BV(INPUT_PIR) ) {	//check if there is someone near
				ADC_start_conversion();	
				while(ADC_done == FALSE) { ; }	//wait until conversion is done;
				ADC_done = FALSE;
				if (is_dark == TRUE) {
					activate_light(FALSE);
				}
				_delay_ms(DEFAULT_DELAY);	
			}
		}
		
		if (light_turned_on == TRUE) {	
			if ( !(PORT_PIR & _BV(INPUT_PIR)) ) {	//check if there's still someone near
				deactivate_light_maybe();			//if there is noone near, turn the lights off
			}
		}

		_delay_ms(DEFAULT_DELAY);	
	}

	return 0;
}
/**************************/
/********** MAIN **********/
/**************************/

void setup() {
	//inputs and outputs
	DDR_STATUS_LED |= _BV(OUTPUT_STATUS_LED);		//status LED pin as output
	PORT_STATUS_LED &= ~_BV(OUTPUT_STATUS_LED);		//status LED turned off
	DDR_PIR &= ~_BV(INPUT_PIR);						//PIR's pin as input
	DDR_CONTROL |= _BV(OUTPUT_CONTROL);				//transistor pin as output
	PORT_CONTROL &= ~_BV(OUTPUT_CONTROL);			//transistor output at zero state

	//PWM
	TCCR0A |= _BV(WGM00);		//Timer0 in Phase Correct PWM mode
	TCCR0A |= _BV(COM0A1);		//without inverted output
	TCCR0B |= _BV(CS00);		//without frequency divider

	//ADC
	ADMUX |= _BV(REFS0);								//AREF as connection with GND by capacitor
	ADMUX |= _BV(MUX0) | _BV(MUX2);						//ADC5 as ADC input
	ADCSRA |= _BV(ADEN) | _BV(ADIE);					//ADC in interruption mode
	ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);		//frequency divider for ADC
	DIDR0 = _BV(ADC5D);									//disabling digital input in ADC pin

	//blink status LED two times to informa about setting finished correctly
	PORTB |= _BV(OUTPUT_STATUS_LED);
	_delay_ms(DEFAULT_DELAY);
	PORTB &= ~_BV(OUTPUT_STATUS_LED);
	_delay_ms(DEFAULT_DELAY);
	PORTB |= _BV(OUTPUT_STATUS_LED);
	_delay_ms(DEFAULT_DELAY);
	PORTB &= ~_BV(OUTPUT_STATUS_LED);
}

void ADC_start_conversion() {
	//every conversion need to be initialised "manually"
	ADCSRA |= _BV(ADSC);
}

void activate_light(uint8_t fast) {
	if (fast == TRUE) {
		//turn the lights on without using gently lightening with PWM
		set_duty(MAX_DUTY);	
	} else {
		//turn the lights on gently using PWM
		for (uint8_t i = current_duty; i < MAX_DUTY; ++i) {
			set_duty(i);
			_delay_ms(PWM_DELAY);
		}
	}
	set_duty(MAX_DUTY);	
	light_turned_on = TRUE;	
}

void deactivate_light_maybe() {
	uint8_t suddenly_activated = FALSE;
	uint8_t i;

	//start dimming the light
	for (i = current_duty; i > 0; --i) {
		set_duty(i);
		_delay_ms(PWM_DELAY);
	
		if ( PORT_PIR & _BV(INPUT_PIR) ) {	//check if someone appeared unexpectedly
			activate_light(TRUE);			//turn the lights on fast, without using PWM
			suddenly_activated = TRUE;
			break;
		}
	}

	if (suddenly_activated == FALSE) {
		set_duty(0);
		light_turned_on = FALSE;
	}
}

void set_duty(uint8_t duty) {
	PWM_DUTY_REGISTER = current_duty = duty;
}
