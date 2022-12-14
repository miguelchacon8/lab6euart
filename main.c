/*
 * File:   main.c
 * Author: pablo
 *
 * Created on October 7, 2022, 8:29 AM
 */

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "oscilador.h"
#include <stdlib.h>
#include <string.h>
#define _XTAL_FREQ 8000000


void setup(void);
void initUART(void);
void cadena(char *cursor);

void setupADC(void);
void setupPWM(void);

unsigned int ADC_RES; //valor del ADC ccp1
unsigned int valDC;
unsigned int valDCL;
unsigned int valDCH;

char buffer[sizeof(unsigned int)*8 +1];
unsigned int i = 0;
//******************************************************************************
// C??digo Principal
//******************************************************************************
void main(void) {
    
    setup();
    setupINTOSC(7);         // 8 MHz
    initUART();
    setupADC();
    setupPWM();
    
    
    while(1){
        cadena("\rMENU PRINCIPAL");
        cadena("\r Seleccione la opcion presionando el numero");
        cadena("\r 1 - LEER POTENCIOMETRO");
        cadena("\r 2 - ENVIAR VALOR ASCII");
        cadena("\r");
        
        
        while (PIR1bits.RCIF == 0);
        if(RCREG == 0b00110001){  //selecci??n del valor del pot
        cadena("Valor del potenciometro \r");
        ADCON0bits.CHS = 0b0001; //se lee el channel 1
        __delay_us(100); 
        ADCON0bits.GO = 1;
        while(ADCON0bits.GO == 1){
        
        }
        ADIF = 0; 
        ADC_RES = ((ADRESH<<2)+(ADRESL>>6)); // le mapeo los valores
        cadena(utoa(buffer, ADC_RES, 10));
        cadena("\r");
        __delay_ms(1); 
        }
        
        
        if(RCREG == 0b00110010){ 
          cadena("INGRESE VALOR DE ASCII \r");
          while (PIR1bits.RCIF == 0);
          if(PIR1bits.RCIF == 1){
              PORTD = RCREG;
              TXREG = RCREG;
              PIR1bits.RCIF = 0;
           }
        }
        
        //if(TXSTAbits.TRMT == 1){
            //RCREG = PORTD;
        //cadena("hola");
//            PORTD++;
//            if(TXSTAbits.TRMT == 1){
//            TXREG = PORTD;
//            }
//            if(PIR1bits.RCIF == 1){
//            PORTB = RCREG;
//            PIR1bits.RCIF = 0;
//            }
            __delay_ms(500);
    }
}
//******************************************************************************
// Funci??n para configurar GPIOs
//******************************************************************************
void setup(void){
    ANSELH = 0;
    TRISB = 0;
    TRISD = 0;

    PORTB = 0;
    PORTD = 0;
}

//******************************************************************************
// Funci??n para configurar UART
//******************************************************************************
void initUART(void){
    // Paso 1: configurar velocidad baud rate
    
    SPBRG = 12;
    
    // Paso 2:
    
    TXSTAbits.SYNC = 0;         // Trabajaremos de forma asincrona
    RCSTAbits.SPEN = 1;         // habilitamos m??dulo UART
    
    // Paso 3:
    // Saltamos porque no usamos 9 bits
    
    // Paso 4:
    TXSTAbits.TXEN = 1;         // Habilitamos la transmision
    
    PIR1bits.TXIF = 0;
    
    RCSTAbits.CREN = 1;         // Habilitamos la recepcion
    
}

//******************************************************************************
//FUNCI??N DE CARACTER
//******************************************************************************
void cadena(char *cursor){
    while (*cursor != '\0'){
        while (PIR1bits.TXIF == 0);
            TXREG = *cursor;
            cursor++;
    } 
}

//******************************************************************************
// Funci??n para configurar ADC
//******************************************************************************
void setupADC(void){   
    // Paso 1 Seleccionar puerto de entrada
    //TRISAbits.TRISA0 = 1;
    ANSELH = 0;
    TRISAbits.TRISA1 = 1;
    ANSELbits.ANS1 = 1;
    // Paso 2 Configurar m??dulo ADC
    //ADRESH=0;		/* Flush ADC output Register */
    //ADRESL=0;
    
    ADCON0bits.ADCS1 = 0;
    ADCON0bits.ADCS0 = 1;       // Fosc/ 8
    
    ADCON1bits.VCFG1 = 0;       // Ref VSS
    ADCON1bits.VCFG0 = 0;       // Ref VDD
    
    ADCON1bits.ADFM = 0;        // Justificado hacia izquierda
    
//    ADCON0bits.CHS3 = 0;
//    ADCON0bits.CHS2 = 0;
//    ADCON0bits.CHS1 = 0;
//    ADCON0bits.CHS0 = 1;        // Canal AN0
    
    ADCON0bits.ADON = 1;        // Habilitamos el ADC
    __delay_us(100);
    
}
//******************************************************************************
// Funci??n para configurar PWM
//******************************************************************************
void setupPWM(void){
    // Paso 1TRISCbits.TRISC1 = 1;  
    // Paso 2
    PR2 = 155;      // Periodo de 20mS  
    // Paso 3
    CCP1CON = 0b00001100;        // P1A como PWM 
    CCP2CONbits.CCP2M = 0b1111;
   // Paso 4
    CCP1CONbits.DC1B = valDCL;        // CCPxCON<5:4>
    CCPR1L = valDCH ;        // CCPR1L   
    
    // Paso 5
    TMR2IF = 0;
    T2CONbits.T2CKPS = 0b11;      // Prescaler de 1:16
    TMR2ON = 1;         // Encender timer 2 
    // Paso 6
    while(!TMR2IF);
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;// Habilitamos la salida del PWM   
}
