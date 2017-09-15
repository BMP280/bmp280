#include <stdint.h>
#include <TM4C1294NCPDT.h>
#define SLAVE_ADDR 0x76 /* 1010 000 */



void I2C1_init(void);
static int I2C_wait_till_done(void);
uint8_t I2C1_byteRead(uint8_t slaveAddr, uint8_t memAddr, uint8_t* data);


char readChar(void);
void printChar(char c);
void printString(char *string);
void hex_to_bcd(uint8_t value, uint8_t *buf);
void bcd_to_ascii(uint8_t *buf);
/*-----------Globals----------*/
uint8_t chip_Id=0, status=0;

int main()
{
	volatile unsigned long delay=0;
	uint8_t bcd[2]={0};
//	char terminal;
  SYSCTL->RCGCGPIO  |= ((0x01)|(1<<12));
  delay = SYSCTL->RCGCGPIO;
  
  
  SYSCTL->RCGCUART |= 0x01;
	GPIOA_AHB->AFSEL  |= ((1<<1)|(1<<0));
  GPIOA_AHB->PCTL = 0x11;
  GPIOA_AHB->DEN = (1<<1)|(1<<0);
  GPION->DIR=0x03;
  GPION->DEN = 0x03;
   
  UART0->CTL &= ~(1<<0);
  UART0->IBRD = 8U;   /* baudrateset for 115200bps */
  UART0->FBRD = 44U;
  UART0->LCRH = 0x00000060;
  UART0->CC = 0x0;
  UART0->CTL = (0x1<<0)|(0x1<<8)|(0x1<<9);

  I2C1_init();
	status = I2C1_byteRead(SLAVE_ADDR, 0xD0, &chip_Id);
	hex_to_bcd(chip_Id, bcd);
	bcd_to_ascii(bcd);
	printString("\n\rChip ID:0x");
	printChar(bcd[1]);
  printChar(bcd[0]);
	printChar('\n');
	printChar('\r');
	return 0;
}

/*Initialize I2C1 as master and port pins*/
void I2C1_init(void)
{
	SYSCTL->RCGCI2C |= 0x02; /*enable clock to I2C1*/
	SYSCTL->RCGCGPIO |= (0x01<<6); /* enable clock to GPIOG*/
	/*PORTG 0, 1 for I2C1 */
	
	GPIOG_AHB->AFSEL |= ((0x01<<0)|(0x01<<1));
	GPIOG_AHB->PCTL &= ~(0x000000FF);
	GPIOG_AHB->PCTL |= 0x00000022;
	GPIOG_AHB->DEN |= (0x03);
	GPIOG_AHB->ODR |= 0x02;
	I2C1->MCR = 0x10;  /* Master Mode*/
	I2C1->MTPR = 7;  /*100kHz @ 16Mhz*/
}

/*wait untill I2C master  is not busy and return error code */
/* if there is no error return 0*/

static int I2C_wait_till_done(void)
{
	while(I2C1->MCS & 1 ); /* wait until I2C  master is not busy*/
	return I2C1->MCS & 0xE; /* return I2C error code*/
	
}

uint8_t I2C1_byteRead(uint8_t slaveAddr, uint8_t memAddr, uint8_t* data)
{
	uint8_t error;
	uint32_t counter;
	/*send slave address and starting address*/

	I2C1->MSA = slaveAddr<<1;
	I2C1->MDR = memAddr;
	I2C1->MCS = 3; /* S-(saddr+w)-ACK-memAdddr-ack */
	for(counter=0; counter<3000000; counter++)
	{
	}
	error = I2C_wait_till_done(); /*wait until write is completed*/
	if(error) return error;
	

	/* to change bus from write to read, send restart with slave Addr*/
	I2C1->MSA = (slaveAddr<<1) + 1; /*restart: -R-(saddr+r)-ACK- */
	
	I2C1->MCS = 7; /*  -data-NACK-P*/
	
	error = I2C_wait_till_done(); /*wait until write is completed*/
	if(error) return error;
	for(counter=0; counter<3000000; counter++)
	{
	}
	*data = I2C1->MDR; /*store the data recieved*/
	return 0;
	
	
}

char readChar(void)
{
  char c;
  while((UART0->FR & (1<<4)) != 0);
  c=UART0->DR;
  return c;
}

void printChar(char c)
{
  while((UART0->FR & (1<<5)) != 0);
  UART0->DR=c;
}

void printString(char *string)
{
  while(*string)
  {
    printChar(*(string++));
  }
}

void hex_to_bcd(uint8_t value, uint8_t *buf)
{
	buf[0] = (value & 0x0F);
	buf[1] = (((uint8_t)value & (uint8_t)0xF0)>>4);
}

void bcd_to_ascii(uint8_t *buf)
{
	buf[0] = buf[0] + '0';
	buf[1] = buf[1] + '0';
}
