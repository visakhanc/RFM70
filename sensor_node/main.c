#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "avr_spi.h"
#include "rfm70.h"
#include "mpu6050.h"

#define RED_LED					PB0
#define RED_LED_DDR				DDRB
#define RED_LED_PORT			PORTB

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))


void timer0_init(void);
void power_on_delay(void);
void sub_program_1hz(void);

volatile bool flag;
static uint8_t tx_buf[8];
static uint8_t sensor_data[14];
static uint8_t addr[CONFIG_RFM70_ADDR_LEN] = CONFIG_RFM70_ADDRESS;

int main(void)
{	
	power_on_delay();  
	timer0_init();
	rfm70_init(RFM70_MODE_PTX, addr);
	RED_LED_OUT();
	RED_LED_OFF();	
	sei();   // enable interrupts globally

	mpu6050_init();
	while(1)
	{
		sub_program_1hz();
	}
}


/*********************************************************
Function: timer0_init                                       
                                                            
Description:                                                
	initialize timer. 
*********************************************************/
void timer0_init(void)
{
	TCCR0 = 0x5; 		/* Start timer0 at frequecy of 8MHz/1024 = 128us */
	TIMSK |= (1 << 0); 	/* Enable Interrupt for timer0 overflow */ 
}


/*********************************************************
Function: Timer0 ISR                                        
                                                            
Description:                                                
 
*********************************************************/
ISR(TIMER0_OVF_vect)
{
 	static uint8_t count = 0;
	
	count++;
	if(count == 2)
	{
		count = 0;
       	flag = true;
	}
}


/*********************************************************
Function:      power_on_delay()                                    
                                                            
Description:                                                
 
*********************************************************/
void power_on_delay(void)
{
	unsigned int i;
	for(i = 0; i<1000; i++)
	{
		_delay_ms(1);
	}
}



/*********************************************************
Function:  sub_program_1hz()                                        
                                                            
Description:                                                
 
*********************************************************/
void sub_program_1hz(void)
{
	uint8_t i, len;
	
	if(flag == true)
	{
		flag = false;
	
		/* Get MPU6050 Accelerometer data */
		if(0 != mpu6050_get_data(sensor_data, sizeof(sensor_data))) {
			RED_LED_ON();
		}
		else {
			
			tx_buf[0]++;
			tx_buf[7] = 0;
			/* Copy temperature data - bytes 6,7 */
			tx_buf[1] = sensor_data[6];
			tx_buf[2] = sensor_data[7];
			for(i=0;i<7;i++)
			{
				tx_buf[7] += tx_buf[i];
			}
			
			//RED_LED_ON();
			if(0 == rfm70_transmit_packet(tx_buf,sizeof(tx_buf))) {
				RED_LED_TOGGLE();
			}
		}

	}
}



