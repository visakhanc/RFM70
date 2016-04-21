#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "rfm70.h"
#include "usbdrv.h"

#define RED_LED					PC1
#define RED_LED_DDR				DDRC
#define RED_LED_PORT			PORTC

#define RED_LED_OUT()			(RED_LED_DDR |= (1 << RED_LED))
#define RED_LED_ON()			(RED_LED_PORT |= (1 << RED_LED))
#define RED_LED_OFF()			(RED_LED_PORT &= ~(1 << RED_LED))
#define RED_LED_TOGGLE()		(RED_LED_PORT ^= (1 << RED_LED))

void timer0_init(void);
void power_on_delay(void);
void Receive_Packet(void);

static uint8_t rx_buf[CONFIG_RFM70_STATIC_PL_LENGTH];
static uint8_t addr[CONFIG_RFM70_ADDR_LEN] = CONFIG_RFM70_ADDRESS;


usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	uint8_t len;
	
	if(data[1] == 1) { /* Send data in rx_buf */
		usbMsgPtr = rx_buf;
		len = sizeof(rx_buf);
	}
	
	return len;
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

/* Check if a packet is received and get the packet to rx buffer */
void Receive_Packet(void)
{
	uint8_t i, len, chksum = 0, pipe; 
	
	pipe = rfm70_receive_packet(rx_buf, &len);
	if(len == 0) { /* No packet received */
		return;
	}
	
	RED_LED_TOGGLE();
		
	i = len-1;
	do {
		i--;
		chksum += rx_buf[i]; 
	} while (i);
	
	if(chksum==rx_buf[len-1]) {
		/* Packet received correctly */
		if(usbInterruptIsReady()) {
			usbSetInterrupt((void *)rx_buf,  sizeof(rx_buf));  // send whole data in interrupt packet
		}
	}
}




int main(void)
{	
	uint8_t 	i;
	
	power_on_delay();  
	rfm70_init(RFM70_MODE_PRX, addr);
	RED_LED_OUT();
	RED_LED_OFF();
	
	usbInit();
	usbDeviceDisconnect();  /* Disconnect for some time to enforce reenumeration */
	i = 0;
	while(--i) {
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();   // enable interrupts globally
	
	while(1)
	{
		usbPoll();
		Receive_Packet();
	}
}

