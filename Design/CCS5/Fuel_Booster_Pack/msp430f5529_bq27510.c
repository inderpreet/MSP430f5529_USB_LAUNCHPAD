/*
* Description: This file is part of the modified demo for the MSP430 Fuel Booster Pack
* --
* Copyright (C) 2014 Inderpreet Singh(er.inderpreet@gmail.com),
			  Thought Process Designs
* Web      :  http://google.com/+InderpreetSingh
*		 	  http://embeddedcode.wordpress.com
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL2) as published by the Free Software
* Foundation and appearing in the file LICENSE.TXT included in the packaging of
* this file. Please note that GPL2 Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL2 ("Copyleft").
*
* We put a lot of time and effort into our project and hence this copyright
* notice ensures that people contribute as well as each contribution is
* acknowledged. Please retain this original notice and if you make changes
* please document them below along with your details.
* The latest copy of this project/library can be found at:
* https://github.com/inderpreet/
*
*/
// ----------------------------------------------------------------------------

/* Version 1.1
 * Inderpreet Singh
 * Modified the original code to work with the MSP430f5529.
 *
*/
 
/*
******************************************************************************
*                                  INCLUDE FILES
******************************************************************************
*/
#include <msp430.h>
#include "bq27510.h"

/*
******************************************************************************
*                                  DEFINITIONS
******************************************************************************
*/
#define bq27510_ADR      0x55
#define ATRATE_MA       -100        /* USER CONFIG: AtRate setting (mA) */
#define TX               BIT0       /* define for UART */


/*
******************************************************************************
*                                  GLOBAL VARIABLES
******************************************************************************
*/
char RxDataBuff[20];
unsigned int timerCount = 10;
volatile unsigned char FLAGS = 0;


/*
******************************************************************************
*                                  FUNCTION PROTOTYPES
******************************************************************************
*/
unsigned int transBytes2Int(unsigned char msb, unsigned char lsb);
int USCI_I2C_READ(char *buffer, int, int);
void Setup_TX();
void Setup_RX();
void fputc(unsigned);
void fputs(char *);
void sendByte(unsigned char);
char receiveByte();
void getstring(char *);
extern void printf(char *, ...);
void initTimer(void);
void initUART(void);
void initI2C(void);

/*
******************************************************************************
*                                  FUNCTION DEFINITIONS
******************************************************************************
*/

/**
  * @brief  Main function
  * @param  None
  * @retval None
  */
int main(void)
{
    char temp;
    char str[20];
    char c = '%';
    int interval = 0;
    int i;
    unsigned int valid_data = 0;
    int temperature, voltage;
    signed int  AverageCurrent;
    unsigned int  RemainingCapacity;
    unsigned int  soc;                        /* Stores State of Charge */
    unsigned int  dcap;                       /* Stores Design Capacity */
    WDTCTL = WDTPW + WDTHOLD;                 /* Stop WDT */

    initI2C();
    /* Init UART for print message */
    initUART();

    printf("******************************************\r\n");
    printf("This is a battery demo !				  \r\n");
    printf("Press:									  \r\n");
    printf("1. To see the battery main parameter 	  \r\n");
    printf("2. To see the battery State of charge!	  \r\n");
    printf("******************************************\r\n");
    printf("\r\n");
    printf("\r\n");
    printf("\r\n");

    temp = receiveByte();

    while (1) {
        if (temp == '1') {
            /* Start the timer */
            initTimer();

            while (1) {
            	valid_data = 1;

                /* Read temperature (units = 0.1K) */
                if(!USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_TEMP_LSB)) {
                	/* Convert K to Celsius degree */
                	temperature = (transBytes2Int(RxDataBuff[1], RxDataBuff[0])) / 10 - 273;
                } else {
                	valid_data = 0;
                }
                
                /* Read voltage (units = mV) */
                if(!USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_VOLT_LSB)) {
                	voltage = transBytes2Int(RxDataBuff[1], RxDataBuff[0]);
                } else {
                	valid_data = 0;
                }
                
                /* Read AverageCurrent (units = mA) */
                if(!USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_AI_LSB)) {
                	AverageCurrent = transBytes2Int(RxDataBuff[1], RxDataBuff[0]);
                } else {
                	valid_data = 0;
                }
                
                /* Read state of charge (units = %) */
                if(!USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_SOC_LSB)){
                	soc = transBytes2Int(RxDataBuff[1], RxDataBuff[0]);
                } else {
                	valid_data = 0;
                }
                
                /* Read DesignCapacity (units = mAH) */
                if(!USCI_I2C_READ(RxDataBuff, 2, 0x2e)) {
                	dcap = transBytes2Int(RxDataBuff[1], RxDataBuff[0]);
                } else {
                	valid_data = 0;
                }
                
                /* Read RemainingCapacity (units = mAH) */
                if(!USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_RM_LSB)) {
                	RemainingCapacity = transBytes2Int(RxDataBuff[1], RxDataBuff[0]);
                } else {
                	valid_data = 0;
                }

                /* every 10*0.5s print the battery message */
                if (timerCount >= 10) {
                    /* Clear the console */
                    printf("\033[2J");
                    printf("\033[H");
                    
                    if (valid_data) {
						/* Read AverageCurrent (units = mA) */
						USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_AI_LSB);
						AverageCurrent = transBytes2Int(RxDataBuff[1], RxDataBuff[0]);

						if (AverageCurrent > 0) {
							printf("The battery is charging!\r\n");
						} else {
							printf("The battery is discharging!\r\n");
						}

						timerCount = 0;

						printf("Current Temperature  :%d��\r\n", temperature);
						printf("Current Voltage  :%dmV\r\n", voltage);
						printf("AverageCurrent  :%dmA\r\n", AverageCurrent);
						printf("State of Charge :%d", soc);
						printf("%c\r\n", c);
						printf("DesignCapacity :%dmAH\r\n", dcap);
						printf("RemainingCapacity :%dmAH\r\n", RemainingCapacity);
                    } else {
                    	timerCount = 0;
                    	printf("There is no battery or the battery's capacity is too low\n\r");
                    	printf("Please plugin a battery or charge the battery\n\r");
                    }
                }
            }
        }

        if (temp == '2') {

            printf("Please set the show time interval(units = s)\r\n");

            /* Get your input and store in str */
            getstring(str);

            /* Convert the string into int */
            for (i = 0; str[i] != '\r'; i++) {
                interval = interval * 10 + str[i] - '0';
            }

            /* Star the timer */
            initTimer();
            printf("\r\n");
            timerCount = interval * 2;

            while (1) {
                /* Read state of charge (units = %) */
            	USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_SOC_LSB);
            	USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_SOC_LSB);

                if(!USCI_I2C_READ(RxDataBuff, 2, bq27510CMD_SOC_LSB)) {
                	soc = transBytes2Int(RxDataBuff[1], RxDataBuff[0]);
                } else {
                	 if (timerCount >= interval * 2) {
                	     timerCount = 0;
                	     /* Clear the console */
                	     printf("\033[2J");
                	     printf("\033[H");
                	     printf("There is no battery or the battery's capacity is too low\n\r");
                	     printf("Please plugin a battery or charge the battery\n\r");
                	     continue;
                	 }
                }

                /* Show the state of charge every interval*2*0.5s */
                //if (timerCount >= interval * 2) {
                {
                    timerCount = 0;
                    printf("State of Charge :%d", soc);
                    printf("%c\r\n", c);

                }

                if (soc == 100) {
                    printf("State of Charge :%d", soc);
                    printf("%c\r\n", c);
                    temp = '0';
                    break;
                }
            }
        }
    }
}


/**
  * @brief  Translate two bytes into an integer
  * @param  
  * @retval The calculation results
  */
unsigned int transBytes2Int(unsigned char msb, unsigned char lsb)
{
    unsigned int tmp;

    tmp = ((msb << 8) & 0xFF00);
    return ((unsigned int)(tmp + lsb) & 0x0000FFFF);
}


/**
  * @brief  Generate I2C stop condition
  * @param  
  * @retval None
  */
void i2c_stop(void)
{
	int i;
	/* I2C stop condition */
	UCB0CTL1 |= UCTXSTP;

	/* Ensure stop condition got sent */
    for (i=0; i<1000; i++) {
    	if ((UCB0CTL1 & UCTXSTP) == 0)
    		break;
    }

    P1OUT |= BIT0;
}


/**
  * @brief  Generate I2C start condition
  * @param  
  * @retval None
  */
void i2c_start(void)
{
    UCB0CTL1 |= UCTXSTT;
}


/**
  * @brief  Prepare for i2c transmitting
  * @param  
  * @retval None
  */
void Setup_TX(void)
{
    int i;

    /* Ensure stop condition got sent */
    for (i=0; i<1000; i++) {
    	if ((UCB0CTL1 & UCTXSTP) == 0)
    		break;
    }

    /* Enable SW reset */
    UCB0CTL1 |= UCSWRST;

    /* I2C Master, synchronous mode */
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;

    /* Use SMCLK, keep SW reset */
    UCB0CTL1 = UCTR + UCSSEL_2 + UCSWRST;

    /* fSCL = SMCLK/12 = ~100kHz */
    UCB0BR0 = 12;
    UCB0BR1 = 0;

    /* Slave Address is 055h */
    UCB0I2CSA = bq27510_ADR;

    /* Clear SW reset, resume operation */
    UCB0CTL1 &= ~UCSWRST;
}


/**
  * @brief  Prepare for i2c receiving
  * @param  
  * @retval None
  */
void Setup_RX(void)
{
	/* Enable SW reset */
    UCB0CTL1 |= UCSWRST;

    /* I2C Master, synchronous mode */
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;

    /* Use SMCLK, keep SW reset */
    UCB0CTL1 = UCSSEL_2 + UCSWRST;

    /* fSCL = SMCLK/12 = ~100kHz */
    UCB0BR0 = 12;
    UCB0BR1 = 0;

    /* Slave Address is 055h */
    UCB0I2CSA = bq27510_ADR;

    /* Clear SW reset, resume operation */
    UCB0CTL1 &= ~UCSWRST;

}


/**
  * @brief  Receive data from bq27510 through i2c bus
  * @param  
  * @retval  0 : Operation normal
  *         -1 : Could not read data
  */
int USCI_I2C_READ(char *buffer, int num, int cmd)
{
    char *PRxData;
    int i=0;

    Setup_TX();
    i2c_start();

    /* Send command to bq27510 */
    if (UCB0IFG & UCTXIFG) {
        UCB0TXBUF = cmd;
    }

    for (i=0; i<1000; i++) {
    	if (UCB0IFG & UCTXIFG)
    		break;
    }


    /* I2C could not send command
     * So we could not read data from bq27510
     * So we just return
     */
    if (i == 1000) {
    	return -1;
    }
    /* Start of RX buffer */
    PRxData = buffer;
    UCB0CTL1 &= ~UCTR;

    /* Send restart to read from bq27510 register */
    i2c_start();

    while (num > 0) {
    	for (i=0; i<1000; i++) {
			if (UCB0IFG & UCRXIFG) {
				/* Load TX buffer */
				*PRxData = UCB0RXBUF;

				/* Decrement TX byte counter */
				num--;
				PRxData++;
				UCB0IFG &= ~UCRXIFG;
				break;
			}
    	}

    	if (i == 1000) {
    	   	return -1;
    	}
    }

    i2c_stop();
    /* Clear USCI_B0 TX int flag */
    UCB0IFG &= ~UCTXIFG;

    return 0;
}


/**
  * @brief  Initialize the UART for 9600 baud with a RX interrupt
  * @param  
  * @retval None
  */
void initUART(void)
{

	P4SEL = BIT4+BIT5;                        // P4.4,4.5 = USCI_A1 TXD/RXD

	UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA1CTL1 |= UCSSEL_1;                     // CLK = ACLK : The external Clock crystal I think
    UCA1BR0 = 0x03;                           // 32kHz/9600=3.41
    UCA1BR1 = 0x00;                           //
    UCA1MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
}


/**
  * @briefInitialize the timer A1
  * @param  
  * @retval None
  */
void initTimer(void)
{
    TA1CCTL0 = CCIE;
    /* Set the timer A to SMCLCK, Continuous,input divider 8. */
    TA1CTL = TASSEL_2 + MC_2 + ID_3;
    __enable_interrupt();
    __bis_SR_register(GIE);

}


/**
  * @brief  puts() is used by printf() to display or send a string. This 
  *         function determines where printf prints to. For this case it sends
  *         a string out over UART, another option could be to display the
  *         string on an LCD display.
  * @param  
  * @retval None
  */
void fputs(char *s)
{
    char c;

    /* Loops through each character in string 's' */
    while (c = *s++) {
        sendByte(c);
    }
}


/**
  * @brief  fputc() is used by printf() to display or send a character. This 
  *         function determines where printf prints to. For this case it sends
  *         a character out over UART.
  * @param  
  * @retval None
  */
void fputc(unsigned b)
{
    sendByte(b);
}


/**
  * @brief  Sends a single byte out through UART
  * @param  
  * @retval None
  */
void sendByte(unsigned char byte )
{
	/* USCI_A0 TX buffer ready? */
	//    while (!(IFG2 & UCA0TXIFG));
	while (!(UCA1IFG&UCTXIFG));             // USCI_1 TX buffer ready?
    /* TX -> RXed character */
    UCA1TXBUF = byte;
}


/**
  * @brief  Receive a byte from console
  * @param  
  * @retval c
  */
char receiveByte( )
{
    char c;

    // while (!(IFG2 & UCA0RXIFG));
    while (!(UCA1IFG&UCRXIFG));             // USCI_10 RX buffer ready?

    c = UCA1RXBUF;
    return c;
}


/**
  * @brief  Receive a string from console
  * @param  
  * @retval None
  */
void getstring(char *s)
{
    int i = 0;

    while (1) {
        //while (!(IFG2 & UCA0RXIFG));
    	while (!(UCA1IFG&UCRXIFG));             // USCI_10 RX buffer ready?

        s[i] = UCA1RXBUF;

        if (s[i] == '\r') {
            break;
        } else {
        	/* USCI_A0 TX buffer ready? */
            //while (!(IFG2 & UCA0TXIFG));
        	while (!(UCA1IFG&UCTXIFG));             // USCI_10 RX buffer ready?

            /* TX -> RXed character */
            UCA1TXBUF = s[i];
            i++;
        }
    }
}


/**
  * @brief  Interrupt routine for receiving a character over UCA1RXBUF
  * @param  
  * @retval None
  */
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	char r;
	switch(__even_in_range(UCA1IV,4)){
		case 0:break;                             // Vector 0 - no interrupt
		case 2:                                   // Vector 2 - RXIFG
			r = UCA1RXBUF;
			sendByte(r);
			//UCA1TXBUF = UCA1RXBUF;                  // TX -> RXed character
			break;
		case 4:break;                             // Vector 4 - TXIFG
		default: break;
  }
}

/**
  * @brief  Timer A0 interrupt service routine,execute every 0.5s
  * @param  
  * @retval None
  */
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void)
{
    timerCount++;
}

/**
  * @brief  init routine for
  * @param
  * @retval None
  */
void initI2C(void){
	P3DIR |= BIT0;                            /* P1.0 output */
	P3OUT &= ~BIT0;
	P3SEL |= BIT0 + BIT1;                     /* Assign I2C pins to USCI_B0 */

}
