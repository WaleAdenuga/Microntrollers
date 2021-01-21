/*
 * General Idea and Procedure
 * I interpreted the question as asking for a sawtooth wave with DAC then using
 * the analog values as input for an ADC. A running average is then calculated for a
 * set number of previous values to attain the most accurate result from the conversion
 * 
 * My process was to first initialize timer0 to get a period of .5 seconds and 
 * the DAC using a for loop counting to 31 (since a DAC can take max 5 bits) 
 * to represent a sawtooth wave. I then connected a wire from RA2(DACOUT) to RA3
 * and initialized the ADC so it used RA3 as input channel and also used timer0
 * when timer0 had an interrupt, i wrote to the DAC and simultaneously
 * started the AD conversion, I then used the last 5 numbers received from the 
 * AD to generate a moving average
 * The values from the moving average was then shown on the LEDs
 */

/*
 * Includes
 */
#include <xc.h>
#define _XTAL_FREQUENCY 48000000
#define AVG_LENGTH 5 //take the last couple values to calculate a running average
#define type_function PORTCbits.RC1
#define switch_bit    PORTAbits.RA3

/*
 * Prototypes
 */
void __interrupt (high_priority) high_ISR(void);   //high priority interrupt routine
void __interrupt (low_priority) low_ISR(void);  //low priority interrupt routine, not used in this example
void initChip(void);
void initTimer(void);
void DAC_start(void);
void ADC_start(void);
void calculateAverage(unsigned int time);
void sawtoothZero();
/*
 * Global Variables
 * 
 */
unsigned int index = 1;
unsigned int tooth = 0;
int sawtooths[AVG_LENGTH] = {0};
//unsigned int average = 0;
int adc_value = 0;
int final_value = 0;

/*
 * Interrupt Service Routines
 */
/********************************************************* 
	Interrupt Handler
**********************************************************/
void __interrupt (high_priority) high_ISR(void)
{
    if (INTCONbits.TMR0IF == 1) {
        ADCON0bits.GO = 1; //start AD conversion through output of DAC
        tooth+=index;   //increase linearly by 1 until 31
        if (tooth == 31) {
            tooth = 0; //sawtooth, go back to 0
            sawtoothZero(); //re-initialize sawtooth numbers
        }
        
        VREFCON2bits.DACR = tooth;  //send sawtooth to DACOUT pin
        //ADRESH = VREFCON2bits.DACR; //send values to ADC directly instead if ADC output is not what you expect
        TMR0 = 0x5B8E;  //reset timer0 to initial value
        INTCONbits.TMR0IF = 0; //clear timer0 interrupt bit when finished 
        
    }
    
    if (PIR1bits.ADIF == 1) { //ADC interrupt flag set
        adc_value = ADRESH; //ADC is left justified, the high byte is sufficiently accurate
        calculateAverage(adc_value); //calculate running average
        LATB = final_value;
        PIR1bits.ADIF = 0; //clear ADC interrupt bit when finished
        
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
    DAC_start();
	INTCONbits.GIE= 1; //global interrupt enable bit
    INTCONbits.PEIE = 1; //peripheral interrupt enable bit
    while(1)    //Endless loop
    {

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
    TRISA = 0b11111011; //Define RA2 as output for dacout pin, RA3 for input of ADC
    ADCON1 = 0x00; //AD voltage reference
    ANSELA = 0b00101111; // define analog or digital, defining RA3 as analog input
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
//use timer0 to set up DAC
void initTimer(void) {
    T0CON = 0b00000111; //prescale of 256, 16 bit mode for .5seconds
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1; //initialize global and peripheral interrupts
    INTCONbits.TMR0IE = 1; //enable timer0 interrupt
    INTCONbits.TMR0IF = 0; //clear interrupt flag
    RCONbits.IPEN = 1;
    INTCON2bits.TMR0IP = 1; //make timer0 high priority interrupt
    
    TMR0 = 0x5B8E;
    //calculation for TMR0
    //A 16 bit timer is used, with Fosc/4, period .5 seconds and prescaler 256
    //Period = (4/Fosc) * Prescaler * Loadvalue
    //= (.5) / ((4/48M) * 256) = 23438, initialize timer with 0x5B8E
    
    T0CONbits.TMR0ON = 1;
}

/*************************************************
			Initialize the DAC
**************************************************/
void DAC_start(void) {
    VREFCON1 = 0b11100000;
    VREFCON2 = 0b00111111;
}

/*************************************************
            Create an ADC, if needed
**************************************************/
void ADC_start(void) {
    ADCON0 = 0b00001101; //ADCON is to select channel3 (RA3) connected to sample and hold circuit
    ADCON1 = 0b00000000; //last 4 bits(3-0) reserved for internal conditions for voltage
    ADCON2 = 0b00001100; //to select acquisition time
    
    PIR1bits.ADIF = 0; //clear the ADC interrupt flag initially, must be cleared by software
    PIE1bits.ADIE = 1; //ADC interrupt enable bit
    
    RCONbits.IPEN = 1;//interrupt priority is enabled
    IPR1bits.ADIP = 1; //high priority interrupt for ADC, IPE has been enabled
    ADCON0bits.ADON = 1;
}

/*************************************************
            Create an AVERAGE, if needed
**************************************************/

void calculateAverage(unsigned int time) {
    unsigned int avg = 0;
    for (int i = AVG_LENGTH-1;i>0;i--) { //moves items from top of list to bottom
        sawtooths[i] = sawtooths[i-1];
        avg += sawtooths[i];
    }
    sawtooths[0] = time; //each new item is the beginning of the list
    avg+=time;
    
    final_value = avg/AVG_LENGTH; //final most accurate value
    
}

void sawtoothZero() {
    for (int i = AVG_LENGTH-1;i>0;i--) {
        sawtooths[i] = 0;
    }
    sawtooths[0] = 0;   //dac reaches its maximum, re-initialize the list of numbers
}


