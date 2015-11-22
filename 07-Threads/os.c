#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)
#define USART_FLAG_RXNE ((uint16_t) 0x0020)
void usart_init(void)
{
	*(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
	*(RCC_APB1ENR) |= (uint32_t) (0x00020000);

	/* USART2 Configuration, Rx->PA3, Tx->PA2 */
	*(GPIOA_CRL) = 0x00004B00;
	*(GPIOA_CRH) = 0x44444444;
	*(GPIOA_ODR) = 0x00000000;
	*(GPIOA_BSRR) = 0x00000000;
	*(GPIOA_BRR) = 0x00000000;

	*(USART2_CR1) = 0x0000000C;
	*(USART2_CR2) = 0x00000000;
	*(USART2_CR3) = 0x00000000;
	*(USART2_CR1) |= 0x2000;
}

void print_str(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

char get_char()
{
	while(1) {
		if ((*USART2_SR) & USART_FLAG_RXNE) 
			return *(USART2_DR) & 0xff;
	}
}

void command(char *cmd)
{

}

void shell(void *data)
{
	char buffer[256];
	while(1)
	{
		int i;
		for (i=0;i<256;i++)
		{
			buffer[i]=get_char();	
			print_str(buffer+i);	
			/* enter command */
			if(buffer[i]==13)		
			{
				buffer[i+1] = '\0';
				print_str("\n");
				command(buffer);
				
				for(;i>0;i--)
					buffer[i]='\0';
				break;
			}
		}	
	}
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{

	usart_init();
	const char *data="shell thread";
	if ( thread_create( shell, (void *) data) == -1 )
		print_str(" shell create failed \r\n ");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
