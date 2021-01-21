
;The idea behind this was to add dividend, divisor and quotient to a file register
;And perform a subtraction between the first two numbers, updating the memory location
;of the dividend, making it suitable for the reminder.
;Everytime a call to division is made, the contents of a memory location containing
;the quotient is incremented, both values are then sent to a loop
;and based on the status of RC0, displays either one
    
;For a division to take place, two checks had to be performed
;First was to make sure the dividend is not greater than zero
;Second was to make sure the dividend (or now remainder after subsequent subtractions)
;is greater than the divisor
    
;Unfortunately, this does not work perfectly, because I was never able to get the loops
;running for long periods
;perhaps a solution would have been to use a timer, and call the loops everytime the timer 
;had an interrupt, but that could also be too fast or too slow
    
;***********************************************************
; File Header
;***********************************************************
    list p=18F25k50, r=hex, n=0
    #include <p18f25k50.inc>	; file to provide register definitions for the specific processor (a lot of equ)

; File registers
    X1 equ  0x00;
 X2 equ	0x00;
;***********************************************************
; Reset Vector
;***********************************************************

    ORG     0x1000	; Reset Vector
			; When debugging:0x0000; when loading: 0x1000
    GOTO    START

;***********************************************************
; Interrupt Vector
;***********************************************************

    ORG     0x1008	; Interrupt Vector HIGH priority
    GOTO    inter_high	; When debugging:0x008; when loading: 0x1008
    ORG     0x1018	; Interrupt Vector LOW priority
    GOTO    inter_low	; When debugging:0x0008; when loading: 0x1018

;***********************************************************
; Program Code Starts Here
;***********************************************************

    ORG     0x1020	; When debugging:0x020; when loading: 0x1020

START
    movlw   0x80	; load value 0x80 in work register
    movwf   OSCTUNE		
    movlw   0x70	; load value 0x70 in work register
    movwf   OSCCON		
    movlw   0x10	; load value 0x10 to work register
    movwf   OSCCON2	
    
    clrf    LATA 	; Initialize PORTA by clearing output data latches
    movlw   0xFF 	; Value used to initialize data direction
    movwf   TRISA 	; Set PORTA as output
    movlw   0x00 	; Configure A/D for digital inputs
    movwf   ANSELA	
    movlw   0x00	; Configure comparators for digital input
    movwf   CM1CON0
    clrf    LATB	; Initialize PORTB by clearing output data latches
    movlw   0x00	; Value used to initialize data direction
    movwf   TRISB	; Set PORTB as output
    clrf    LATC	; Initialize PORTC by clearing output data latches
    movlw   0x01	; Value used to initialize data direction
    movwf   TRISC	; Set RC0 as input

    bcf     UCON,3	; to be sure to disable USB module
    bsf     UCFG,3	; disable internal USB transceiver

main
    clrf    X1
    clrf    X2
    movlw   0x00
    movwf   X1		;value to be divided initialized
    movlw   0x00	
    movwf   X2		;value to divide it with initialized 
    
    ;call    loop
    call    init_timer
    call    init_numbers
    call    manipulate_division1
    ;call    loop

    
    bsf	    INTCON,GIE	;enabling global and peripheral interrupts
    bsf	    INTCON,PEIE

    
init_numbers
    lfsr    0,0x010
    movlw   D'87'	;dividend, used in calculate_division
    movwf   0x010	
    movlw   D'10'	;divisor, memory address used in calculate_division
    movwf   0x011
    movlw   D'0'	;quotient 
    movwf   0x012
    return

loop
    btfss   PORTC,0	    ;checking to see if RC0 is pushed, if not, show quotient
    goto    show_quotient  ;if pushed, show remainder
    goto    show_remainder  
    ;return

manipulate_division1
    
    movlw   0x00
    cpfsgt  0x010	;check if dividend is greater than 0, if yes, continue division, if no, show results
    goto    loop
    call    manipulate_division2
       
    goto    manipulate_division1
    
manipulate_division2
    movff   0x011,W	;divisor to working register
    cpfsgt  0x010	;compare what is left in dividend(remainder) to divisor, if it's greater, skip else continue the subtraction
    goto    loop	;show remainder or quotient if division is done 
    call    division
    return
division
    
    movff   0x011,W	;move the divisor to the working register
    subwf   0x010,1	;subtract divisor from dividend and store in file register
    incf    0x012	;increment quotient because you can still divide
    
    return

    
show_quotient
    movff   0x012,PORTB	 ;how many times have I been able to delete this
    return
    
show_remainder
    movff   0x010,PORTB	;reminder because subtraction cannot be negative, what's left in the file register with the dividend
    return
    

init_timer
    movlw   0x47
    movwf   T0CON	    ;using timer0 in 8 bit mode and prescaler 256
    bsf	    INTCON,GIE	    ;enable global and peripheral interrupts
    bsf	    INTCON,PEIE
    bsf	    INTCON,TMR0IE   ;enable timer0 interrupt
    bcf	    INTCON,TMR0IF   ;clear initial timer0 flag
    bsf	    RCON,IPEN
    bsf	    INTCON2,TMR0IP  ;giving timer0 high priority
    movlw   0x00	    ;start the count from 0 to make it even slower
    movwf   TMR0L
    bsf	    T0CON,TMR0ON    ;turn on the timer
    return
    
; interrupt handling
inter_high
    btfsc   INTCON,TMR0IF    ;check the interrupt is from timer2, else skip
    call    manipulate_division1
    RETFIE
    
inter_low
    nop
    RETFIE
    
    END

