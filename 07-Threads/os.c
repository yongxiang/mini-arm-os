#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"
#include "filesystem.h"
#include "romfs.h"

extern const unsigned char _sromfs;
extern const unsigned char _eromfs;
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

int fib(int number)
{
	if(number==0) return 0;
	int result;
	asm volatile("push {r3, r4, r5, r6}");
	asm (	"mov r4, %[num]"::[num] "r" (number));
	asm (
		"mov r6,#0\n"
		"mov r5,#1\n"
	".Loop:\n"
		"add r3,r5,r6\n"
		"mov r6,r5\n"
		"mov r5,r3\n"

		"subs r4,r4,#1\n"
		"bge .Loop"	
	);
	asm ("mov %[fibresult], r3":[fibresult]"=r"(result));
	asm volatile("pop {r3, r4, r5, r6 }	");

	return result;    
}

int atoi(char *str)
{ 
	int val = 0;
	while(*str)
	{    
		val = val * 10 + *str - '0' ;
		str++ ;
	}
	return val;
}
void itoa(int num,char *s)
{
	char temp[10];
	int i = 0; 
	int j = 0;
	while(num>0)
	{       
		temp[i++] = num%10 + '0';
		num = num/10;
	}
	for(;i>0;i--)
		s[j++]=temp[i-1];
	s[j] = '\0';
}
char *strtok(char *str, const char *delim)
{
	static char *last_str;
	if(str==NULL)
		str=last_str;
	if(!str) return NULL;

	char *head=str;

	while(*str)
	{     
		if((*str)==(*delim))
		{
			*str='\0';
			last_str=str+1;	
			return head;
		}
		str++;
	}
	return head;
}
int strcmp(const char *s1, const char *s2)
{
	while(*s1 && *s2)
	{
		if(*s1 != *s2) return 0;
		s1++; s2++;
	}
	if(*s1=='\0' && *s2=='\0') return 1;
	else return 0;
}

void command(char *cmd)
{
	char *command = strtok(cmd," "); 
	if ( strcmp(command,"fib") )
	{    
		int num = atoi(strtok(NULL," "));
        	int ans = fib(num);
      		char result[10]; 
        	itoa(ans,result);
        	print_str("fib answer: ");
        	print_str(result);
        	print_str("\n");
	}
}


void shell(void *data)
{
	char buffer[10];
	while(1)
	{
		int i;
		for (i=0;i<10;i++)
		{
			buffer[i]=get_char();	
			print_str(buffer+i);	
			/* enter command */
			if(buffer[i]==13)		
			{
				buffer[i] = '\0';  
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
	fs_init();
	register_romfs("romfs",&_sromfs);

	const char *data="shell thread";

	int dir;
	dir = fs_opendir("/romfs/");
	(void) dir;

	if ( thread_create( shell, (void *) data) == -1 )
		print_str(" shell create failed \r\n ");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
