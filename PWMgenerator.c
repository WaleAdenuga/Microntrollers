/*********************************************************************************************************************
 *
 * FileName:        main.c
 * Processor:       PIC18F2550 / PIC18F2553
 * Compiler:        MPLABÃÂ® XC8 v2.00
 * Comment:         Main code
 * Dependencies:    Header (.h) files if applicable, see below
 *
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Author                       Date                Version             Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Eva Andries	                12/10/2018          0.1                 Initial release
 * Eva Andries					 6/11/2018			1.0					XC8 v2.00 new interrupt declaration
 * Tim Stas                     12/11/2018          1.1                 volatile keyword: value can change beyond control of code section
 * Tim Stas						15/07/2019			2.0					PIC18F25K50
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * TODO                         Date                Finished
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *********************************************************************************************************************/

/*
 * Includes
 */
#include <xc.h>
#define _XTAL_FREQUENCY 48000000

#define CHANGE_SWITCH PORTBbits.RB5

/*
 * Prototypes
 */
void __interrupt (high_priority) high_ISR(void);   //high priority interrupt routine
void __interrupt (low_priority) low_ISR(void);  //low priority interrupt routine, not used in this example
void initChip(void);
void initTimer(void);
void initPWM(void);

/*
 * Global Variables
 */
volatile int counter = 14;
char index; //-1 & 1 for triangle
unsigned char duty;
char signal_type; //1 is for sawtooth

/*
 * Interrupt Service Routines
 */
/********************************************************* 
	Interrupt Handler
**********************************************************/
void __interrupt (high_priority) high_ISR(void)
{
	if(PIR1bits.TMR2IF == 1) //that's the interrupt flag being set
     {//but this is a timer interrupt
         
        if (signal_type == 1) { //signal is sawtooth, end of cycle, go immediately to 0
            if (duty == 74) duty = 0;
        } else { //signal is triangle, increase by 1, then reduce by 1
            if (duty == 74) index = -1;
            if (duty == 0) index = 1;
        }
        duty += index;
        CCPR2L = duty;
        TMR2 = 0x00;    			//reload the value to the Timer2
        PIR1bits.TMR2IF=0;     //CLEAR interrupt flag when you are done!!!
     }
    
    if (CHANGE_SWITCH == 1) {
        if (signal_type == 1) { //signal is sawtooth, change to triangle when switch active
            signal_type = 0;
            //index = 1;
        }
        else { //signal is triangle, change to sawtooth
            signal_type = 1;
            index = 1;
        }
    }
}

/*
 * Functions
 */
 /*************************************************
			Main
**************************************************/
void main(void)
{
    initChip();
    initTimer();
    initPWM();
    INTCONbits.GIE = 1;
    
    index = 1;
    signal_type = 1;
    duty = 0;
	
    while(1)    //Endless loop
    {
        
        //LATB = counter ;    //Give value to PORTB

    }
}

/*************************************************
			Initialize the CHIP
**************************************************/
void initChip(void)
{
	//CLK settings
	OSCTUNE = 0x80; //3X PLL ratio mode selected
	OSCCON = 0x70; //Switch to 16MHz HFINTOSC
	OSCCON2 = 0x10; //Enable PLL, SOSC, PRI OSC drivers turned off
	while(OSCCON2bits.PLLRDY != 1); //Wait for PLL lock
	ACTCON = 0x90; //Enable active clock tuning for USB operation

    LATA = 0x00; //Initial PORTA
    TRISA = 0xFF; //Define PORTA as input
    ADCON1 = 0x00; //AD voltage reference
    ANSELA = 0b00001111; // define analog or digital, last 4 pins as analog input, for verification with scope
    CM1CON0 = 0x00; //Turn off Comparator
    LATB = 0x00; //Initial PORTB
    TRISB = 0b00000000; //Define PORTB as output
    LATC = 0x00; //Initial PORTC
    TRISC = 0x00; //Define PORTC as output
	INTCONbits.GIE = 0;	// Turn Off global interrupt
}

/*************************************************
			Initialize the TIMER
**************************************************/
void initTimer(void)
{
    T0CON =0x47;        //Timer0 Control Register
               		//bit7 "0": Disable Timer
               		//bit6 "1": 8-bit timer
               		//bit5 "0": Internal clock
               		//bit4 "0": not important in Timer mode
               		//bit3 "0": Timer0 prescale is assigned
               		//bit2-0 "111": Prescale 1:256  
    /********************************************************* 
	     Calculate Timer 
             F = Fosc/(4*Prescale*number of counting)
	**********************************************************/

    
    TMR0L = 0x00;    //Initialize the timer value
    

    /*Interrupt settings for Timer0*/
    INTCON= 0x20;   /*Interrupt Control Register
               		//bit7 "0": Global interrupt Enable
               		//bit6 "0": Peripheral Interrupt Enable
               		//bit5 "1": Enables the TMR0 overflow interrupt
               		//bit4 "0": Disables the INT0 external interrupt
               		//bit3 "0": Disables the RB port change interrupt
               		//bit2 "0": TMR0 Overflow Interrupt Flag bit
                    //bit1 "0": INT0 External Interrupt Flag bit
                    //bit0 "0": RB Port Change Interrupt Flag bit
                     */
    
    T0CONbits.TMR0ON = 1;  //Enable Timer 0
    INTCONbits.GIE = 0;    //Enable interrupt
}

void initPWM(void) {//PWM uses timer 2
    PR2 = 0b01001010; //prescaler value with timer prescale 16, and PWM frequency being 10kHz    
    CCP2CON = 0b00001100; //PWM mode selected
    
    PIE1 = 0b00000010; //enable bit for timer2
    
    PIR1bits.TMR2IF = 0;//Timer 2 interrupt flag
    
    T2CON = 0b00000110; //1; //Timer 2 has prescale value of 16 and turning it on
    TRISCbits.RC1 = 0; //making RC1 the output of the PWM
    //INTCONbits.RBIE = 1;
    //T2CONbits.TMR2ON = 1;
    
    
}

