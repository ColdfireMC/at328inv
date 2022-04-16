#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <math.h>
#include <util/delay.h>
#include <stdint.h>
#define F_CPU 20000000UL
#define DEFAULT_STEP 1
#define DEFAULT_INC 65536UL*5000UL
#define OF_COMP 16
#define MAX_PHASE_INC 581939200UL
#define MIN_PHASE_INC 4096UL
volatile uint32_t phase_inc=DEFAULT_INC;
volatile uint8_t gain=1;
volatile int button_input=0xFFFF;
volatile bool set_button=0;
volatile bool gain_lock=0;
void pwm_setup(void);
void timer_setup(void);
uint32_t get_tick_count(void);


const __flash uint16_t sine[256]={511, 524, 536, 549, 561, 574, 586, 599, 611, 623, 635, 648, 660, 672, 683, 695, 707, 718, 730, 741, 752, 763, 774, 785, 795, 806, 816, 826, 836, 845, 855,
	864, 873, 882, 890, 899, 907, 915, 922, 930, 937, 944, 950, 956, 963, 968, 974, 979, 984, 989, 993, 997, 1001, 1004, 1008, 1011, 1013, 1015, 1017, 1019, 1021, 1022, 1022, 1023, 1023,
	1023, 1022, 1022, 1021, 1019, 1017, 1015, 1013, 1011, 1008, 1004, 1001, 997, 993, 989, 984, 979, 974, 968, 963, 956, 950, 944, 937, 930, 922, 915, 907, 899, 890, 882, 873, 864, 855,
	845, 836, 826, 816, 806, 795, 785, 774, 763, 752, 741, 730, 718, 707, 695, 683, 672, 660, 648, 635, 623, 611, 599, 586, 574, 561, 549, 536, 524, 512, 499, 487, 474, 462, 449, 437,
	424, 412, 400, 388, 375, 363, 351, 340, 328, 316, 305, 293, 282, 271, 260, 249, 238, 228, 217, 207, 197, 187, 178, 168, 159, 150, 141, 133, 124, 116, 108, 101, 93, 86, 79, 73, 67,
	60, 55, 49, 44, 39, 34, 30, 26, 22, 19, 15, 12, 10, 8, 6, 4, 2, 1, 1, 0, 0, 0, 1, 1, 2, 4, 6, 8, 10, 12, 15, 19, 22, 26, 30, 34, 39, 44, 49, 55, 60, 67, 73, 79, 86, 93, 101, 108,
    116, 124, 133, 141, 150, 159, 168, 178, 187, 197, 207, 217, 228, 238, 249, 260, 271, 282, 293, 305, 316, 328, 340, 351, 363, 375, 388, 400, 412, 424, 437, 449, 462, 474, 487, 499};
bool mask[256]={[0 ... 255]=0};
/*
const uint8_t sine[256]={127, 130, 133, 136, 140, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173, 176, 179, 182, 185, 187, 190, 193, 195, 198, 201, 203, 206, 208, 211, 213, 215, 218, 220, 222, 224, 226, 228, 230, 232, 233, 235, 237, 238, 240, 241, 243, 244, 245, 246, 248, 249, 249, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 249, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 233, 232, 230, 228, 226, 224, 222, 220, 218, 215, 213, 211, 208, 206, 203, 201, 198, 195, 193, 190, 187, 185, 182, 179, 176, 173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 140, 136, 133, 130, 128, 125, 122, 119, 115, 112, 109, 106, 103, 100, 97, 94, 91, 88, 85, 82, 79, 76, 73, 70, 68, 65, 62, 60, 57, 54, 52, 49, 47, 44, 42, 40, 37, 35, 33, 31, 29, 27, 25, 23, 22, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 6, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 6, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 22, 23, 25, 27, 29, 31, 33, 35, 37, 40, 42, 44, 47, 49, 52, 54, 57, 60, 62, 65, 68, 70, 73, 76, 79, 82, 85, 88, 91, 94, 97, 100, 103, 106, 109, 112, 115, 119, 122, 125};
*/
int main(void)
{
	OCR1A=0;
	DDRC=0xFF;
	DDRD=0;
	DDRB=0xFF;
	
	OCR1A=1;
	pwm_setup();
	timer_setup();
	PORTB=0;
	sei();
	PORTD=0;
	
while (1)
{
	if (set_button==1)
	{
	set_button=0;
	switch (button_input)
	{
		case 0b11111110: 
		if (phase_inc<=MAX_PHASE_INC)
		{
			phase_inc+=4096;
		}
		break;		
		
		case 0b11111101:
		if(phase_inc>=MIN_PHASE_INC)
		{
		   phase_inc-=4096;
		} 
		break;
			
		case 0b11111011:
		if(((gain<=10) & gain_lock==0))
		{
		   gain+=1;
		   gain_lock=1;
		} 
		break;
			
		case 0b11110111: 
		if(((gain>1) & gain_lock==0))
		{
		  gain-=1;
		  gain_lock=1;
		} 
		break;
	}
   }
}
	return 0;
}


ISR(TIMER1_OVF_vect)
{
	static uint16_t next_reg = 0;
	static uint32_t phase_acc = 0;
	static uint8_t next_port_out = 0;
	
	PORTC=next_port_out;
	OCR1A=next_reg;
	phase_acc += phase_inc;
	next_reg=sine[phase_acc>>24]/gain;
	next_port_out=((phase_acc>>31) | (mask[phase_acc>>24]<<2) | 1 << 3);
}
ISR(TIMER1_COMPA_vect)
{

}

ISR(TIMER0_COMPA_vect)
{
	static uint8_t sample_array[OF_COMP]={[0 ... OF_COMP-1]=0};
    static uint8_t sample_count=0;
	static uint16_t gain_lock_counter=0;
	uint8_t index;
	bool last_comp;
	
	if (sample_count<=OF_COMP)
	{
		sample_array[sample_count]=PIND;
	    sample_count+=1;
	} 
	else
	{
		last_comp=(sample_array[0]==sample_array[1]);
		for (index=1; index < OF_COMP-1; index++)
		{
			last_comp=(last_comp&(sample_array[index-1]==sample_array[index]));
		}
		if (last_comp==1)
		{
			set_button=1;
			button_input=sample_array[OF_COMP-1];
		}
		sample_count=0;
	}
	if (gain_lock_counter<32768)
	{
		gain_lock_counter++;
	}
	else
	{
		gain_lock_counter=0;
		gain_lock=0;
	}
}
void timer_setup(void)
{
	TIMSK0=(1<<OCIE0A);
	TCCR0A |= (1 << COM1A1);
	TCCR0A |= (1 << WGM11) | (0 << WGM00) | (1<<COM0A0) | (0<<COM1A0);
	TCCR0B |= (1 << CS10) | (0 << CS11) | (0 << CS12) | (0 << WGM02);
	OCR0A=8;
	//OCR0B=64;
}
void pwm_setup(void)
{
	TIMSK1|=(1<<OCIE1A);
	TIMSK1|=(1<<TOIE1);
	TCCR1A |= (1 << COM1A1);
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	TCCR1B |= (0 << CS10) | ( 1 << CS11) | (0 << CS12) | (0 << WGM13) | (0 << WGM12);
}
