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

/*
 * Prototypes
 */
void __interrupt (high_priority) high_ISR(void);   //high priority interrupt routine
void __interrupt (low_priority) low_ISR(void);  //low priority interrupt routine, not used in this example
void initChip(void);
void initTimer(void);
void ADC_start(void);

/*
 * Global Variables
 */
volatile int counter = 14;
int adc_value=0;

/*
 * Interrupt Service Routines
 */
/********************************************************* 
	Interrupt Handler
**********************************************************/
void __interrupt (high_priority) high_ISR(void)
{
	if(INTCONbits.TMR0IF == 1) //that's the timer interrupt flag being set
     {//but this is a timer interrupt
        //LATB = 0xFF;
        ADCON0bits.GO = 1; //start the ADC conversion
        TMR0L = 0x00;    			//reload the value to the Timer0
         
        INTCONbits.TMR0IF=0;     //CLEAR interrupt flag when you are done!!!
     } 
    
    if (PIR1bits.ADIF == 1){ //ADC flag is set
        adc_value = ADRESH; //reading the value from the conversion
        PIR1bits.ADIF = 0; //clear ADC interrupt flag when done
        //ADCON0bits.GO = 1;
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
    ADC_start();
	INTCONbits.GIE= 1; //global interrupt enable bit
    while(1)    //Endless loop
    {
        
        LATB = adc_value;    //Give value to PORTB

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
    ANSELA = 0x0F; // define analog or digital, defining RA0 as analog input
    CM1CON0 = 0x00; //Turn off Comparator
    LATB = 0x00; //Initial PORTB
    TRISB = 0x00; //Define PORTB as output
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
    
    INTCONbits.GIE = 1;    //Enable interrupt
}

void ADC_start(void) {
    ADCON0 = 0b00000001;//ADCON0 is to select channel connected to sample and hold circuit
    ADCON1 = 0b00000000; //ADCON1 is to select 
    //ADCON last 0000 is reserved for internal conditions
    ADCON2 = 0b00111110; //ADCON2 is to select acquisition time
    PIR1bits.ADIF = 0; //Clearing the ADC interrupt flag, must be cleared by software 
    PIE1bits.ADIE = 1; //ADC interrupt enable bit
    
    //INTCONbits.GIE= 1;
    //INTCONbits.PEIE = 1;
    //PIE1bits.ADIE = 1;
    RCONbits.IPEN = 1; //interrupt priority is enabled
    IPR1bits.ADIP = 1; //high priority interrupt for ADC, IPEN has to be enabled
    
    //GIE is replaced by GIEH and GIEL
    //ADCON0bits.ADON = 1; //the ADON bit of the ADCON0 register must be set to 1 to enable the ADC module
    
}
