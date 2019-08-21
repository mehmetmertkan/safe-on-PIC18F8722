#include <p18cxxx.h>
#include <p18f8722.h>
#pragma config OSC = HSPLL, FCMEN = OFF, IESO = OFF, PWRT = OFF, BOREN = OFF, WDT = OFF, MCLRE = ON, LPT1OSC = OFF, LVP = OFF, XINST = OFF, DEBUG = OFF
#define _XTAL_FREQ 40000000

#include "Includes.h"
#include "LCD.h"

// Ilk ADRESi tut, o de?i?irse lcddeki de?eri de?i?tir
void _mydelay();
void initvalues();
void checkadc();
void verysafeyaz();
void pinstateyaz();
void rakamyaz();
void mydelay();
void newstateyaz();
int basildimiPORTE1();
int pickNumber();
void enterpinyaz();
int checkpass();
void tryyaz();
void safeopenyaz();
void sevenseg(int time);
char pass[4];
char enteredpass[4];
int karemi = 1;
int toggle;
int adctoggle = 0;
int cursor = 0x8B; // bir rakam yaz?ld???nda bunu artt?r
int counter = 0;
int adcDone,adcValue;
unsigned char PORTBReg;
int oldadcvalue;
int setbitti;
int enterbitti = 0;
int blinknewpin = 0;
int attempts = 2;
int trytime = 0;
int sevsegon = 0;
int sevseg[11] = {0X3F,0X06,0X5B,0X4F,0X66,0X6D,0X7D,0X07,0X7F,0X6F,0x40};
int time = 120;
int countertmr1 = 0;
int starttime = 0;
int programbitti = 0;

void interrupt ISR()
{
    if(TMR0IE && TMR0IF)
    {
        TMR0IF=0;
        counter++;
        if(blinknewpin && counter ==20)
        {
            
            if(toggle == 1)
            {
                toggle = 0;
                ClearLCDScreen();
                /*
                WriteCommandToLCD(0x80);
                WriteStringToLCD("                ");
                WriteCommandToLCD(0xC0);
                WriteStringToLCD("                ");*/
            }
            else if (toggle == 0)
            {
                
                toggle = 1;
                newstateyaz();
            }
            counter = 0;
        }
        else if(trytime == 1 && counter == 800)
        {
            enterpinyaz();
            cursor = 0x8B;
            karemi=1;
            trytime = 0;
            counter = 0;
        }
        else if(counter==10 && (!trytime)&&(!blinknewpin))
        {
            if(karemi){
                if (toggle == 1)
                {
                    
                    toggle = 0;

                    WriteCommandToLCD(cursor);
                    WriteStringToLCD(" ");
                }
                else if (toggle == 0)
                {
                    
                    toggle = 1;
                    WriteCommandToLCD(cursor);
                    WriteStringToLCD("#");
                }   
            }
            counter=0;  //Reset Counter
        }
        
        GODONE = 1;
    }
    else if(TMR1IE && TMR1IF)
    {
        TMR1IF = 0;
        countertmr1++;
        if(countertmr1 == 20)
        {
            time--;
            countertmr1 =0;
        }   
    }
    else if(INTCONbits.RBIE && INTCONbits.RBIF)
    {
        INTCONbits.RBIF = 0;
        PORTBReg = PORTB;
        INTCONbits.RBIF = 0;
        PORTBReg = PORTB;
        int a = (int)PORTBReg; 
        if((!trytime) &&  PORTBReg == 0x9F) // PORTB6
        {
            if(cursor < 0x8E && !karemi){
                cursor++;
                karemi = 1;
            }
            if(!(cursor < 0x8E))
            {
                if(setbitti != 2)
                    setbitti = 1;
                else 
                {
                    enterbitti = 1;
                    checkpass();
                }
            }
        }
        else if((!trytime) &&  setbitti == 1 && PORTBReg == 0x5F) // PORTB7
        {
            blinknewpin = 1;
        }
        else if((!trytime) &&  enterbitti && (setbitti == 2) && PORTBReg == 0x5F)
        {
            if(checkpass())
            {
                safeopenyaz();
            }
            else
            {
                enterbitti = 0;
                cursor = 0x8B;
                attempts--;
                WriteCommandToLCD(0xCB);
                WriteDataToLCD((char)(((int)'0')+attempts));
                WriteCommandToLCD(0x8B);
                WriteStringToLCD("####");
                karemi = 1;
                if(attempts==0)
                {
                    tryyaz();
                    trytime = 1;
                    adctoggle = 0;
                    counter =0;
                }
            }
        }
        RBIF = 0;
    }
    else if(ADIE && ADIF)
    {   
        if(!trytime)
        {
            if(adctoggle == 0){
            adctoggle = 1;
            oldadcvalue = ADRES;
            adcDone=0;
            }
            else
            {
                adcValue = ADRES;
                if((adcValue/100) != (oldadcvalue/100))
                {
                    karemi=0;
                    adcDone = 1;  
                    oldadcvalue = adcValue;
                }
                else
                {
                    adcDone = 0;
                }
            }
        }
        ADIF = 0;
    }
    
}

void main(void)
{
    INTCONbits.PEIE = 0;
    INTCONbits.GIE = 0;
    INTCONbits.RBIE = 0;
    TMR1IE = 0;
    TMR0IE = 0;
    ADIE = 0;
    
    
    RBIE = 0;
    GIE = 0;
    PEIE = 0;
    ADIE = 0;
    TMR0IE = 0;
 
    InitLCD();          // Initialize LCD in 4bit mode
    
    LATE = 0x0;
    PORTE = 0x0;
    TRISE = 0x2;        // Only PORTE,1 is input        
    TRISH=0X00;
    PORTH=0X00;
    TRISJ=0x00;
    PORTJ=0X00;
    TMR1IE = 0;
    ClearLCDScreen();  // Clear LCD screen
    verysafeyaz();
    while(1){
        if(basildimiPORTE1())
        {
            break;
        }
    }
    mydelay();
    
    pinstateyaz();
    
    INTCON = 0;
    INTCON2 = 0;

    TRISH4 = 1;
    ADCON0 = 0b00110000;
    ADCON1 = 0b00000000;
    ADCON2 = 0b10001010;
    
    TMR0IE = 0;
    T0CON = 0b00000001;

    TMR0IE = 1;
    TMR0IF = 0;
    TMR0ON  = 1;
    
    TMR1IE = 0;
    T1CON = 0b11111100;
    TMR1H = 0b01100111;
    TMR1L = 0b01101010;
    
    TMR1IE = 1;
    TMR1IF = 0;
    TMR1ON  = 1;
    
    ADON = 1;
    ADIF = 0;
    ADIE = 1;
    
    TRISB = 0;
    PORTB = 0xFF;
    TRISBbits.TRISB7 = 1;
    TRISBbits.TRISB6 = 1;
                
    INTCONbits.RBIE = 1;
    INTCONbits.RBIF = 0;

    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    initvalues();
    sevenseg(time);
    while(1)
    {
        sevenseg(time);
        checkadc();
        if(blinknewpin == 1)
        {
            counter=0;
            newstateyaz();
            _mydelay();
            blinknewpin = 0;
            setbitti = 2;
            enterpinyaz();
            cursor = 0x8B;
            karemi=1;
            starttime = 1;
            time = 120;
        }
        if(programbitti == 1)
        {
            break;
            programbitti = 0;
        }
    }
}
void checkadc()
{
    if(adcDone)
    {
        rakamyaz();
        adcDone = 0;
    }
}
void pinstateyaz()
{
    sevenseg(time);
    ClearLCDScreen();       
    sevenseg(time);    // Clear LCD screen
    WriteCommandToLCD(0x81);
    WriteStringToLCD("Set a pin:####");
}
void verysafeyaz()
{
    sevenseg(time);
    ClearLCDScreen();
    sevenseg(time);// Clear LCD screen
    WriteCommandToLCD(0x81);   // Goto to the beginning of the first line
    WriteStringToLCD("$>Very  Safe<$");	// Write Hello World on LCD
    WriteCommandToLCD(0xC1); // Goto to the beginning of the second line
    WriteStringToLCD("$$$$$$$$$$$$$$");
}

int basildimiPORTE1()
{
    if(PORTEbits.RE1 == 0)
    {
        while(1)
        {
            if(PORTEbits.RE1 == 1)
            {
                return 1;
            }
        }
    }
    return 0;
}



int pickNumber()
{
    sevenseg(time);
    if(adcValue <= 99)
    {
        return 0;
    }
    if(adcValue <= 199)
    {
        return 1;
    }
    if(adcValue <= 299)
    {
        return 2;
    }
    if(adcValue <= 399)
    {
        return 3;
    }
    if(adcValue <= 499)
    {
        return 4;
    }
    if(adcValue <= 599)
    {
        return 5;
    }
    if(adcValue <= 699)
    {
        return 6;
    }
    if(adcValue <= 799)
    {
        return 7;
    }
    if(adcValue <= 899)
    {
        return 8;
    }
    if(adcValue <= 1024)
    {
        return 9;
    }
}

void rakamyaz()
{
    sevenseg(time);
    WriteCommandToLCD(cursor);
    
    if(setbitti != 2)
    {
        char num = (char)(((int)'0')+pickNumber());
        pass[cursor-0x8B] = num;
        WriteDataToLCD(num);
    }
    else
    {
        char num = (char)(((int)'0')+pickNumber());
        enteredpass[cursor-0x8B] = num;
        WriteDataToLCD(num);
    }
    sevenseg(time);
    
    
}



void newstateyaz()
{
    sevenseg(time);
    ClearLCDScreen();
    WriteCommandToLCD(0x81); 
    WriteStringToLCD("The new pin is");   
    WriteCommandToLCD(0xC3); 
    WriteStringToLCD("---");
    WriteCommandToLCD(0xC6);
    WriteDataToLCD(pass[0]);
    WriteCommandToLCD(0xC7);
    WriteDataToLCD(pass[1]);
    WriteCommandToLCD(0xC8);
    WriteDataToLCD(pass[2]);
    WriteCommandToLCD(0xC9);
    WriteDataToLCD(pass[3]);
    WriteCommandToLCD(0xCA); 
    WriteStringToLCD("---");
    sevenseg(time);
}

void enterpinyaz()
{
    sevenseg(time);
    ClearLCDScreen();           // Clear LCD screen
    sevenseg(time);
    WriteCommandToLCD(0x81);
    WriteStringToLCD("Enter pin:####");
    WriteCommandToLCD(0xC2);
    WriteStringToLCD("Attempts:2");
    
    counter = 0;
}
int checkpass() //if true return 1, else return 0
{
    int c;
    for(c = 0;c<4;c++)
    {
        if(pass[c] != enteredpass[c])
            return 0;
    }
    return 1;
}

void tryyaz()
{
    sevenseg(time);
    ClearLCDScreen();
    sevenseg(time);
    WriteCommandToLCD(0x81);   // Goto to the beginning of the first line
    WriteStringToLCD("Enter pin:XXXX");	// Write Hello World on LCD
    WriteCommandToLCD(0xC0); // Goto to the beginning of the second line
    WriteStringToLCD("Try after 20 sec.");
    attempts=2;
    
}


void safeopenyaz()
{
    sevenseg(time);
    ClearLCDScreen();
    sevenseg(time);
    // Clear LCD screen
    WriteCommandToLCD(0x80);   // Goto to the beginning of the first line
    WriteStringToLCD("Safe is opening!");	// Write Hello World on LCD
    WriteCommandToLCD(0xC0); // Goto to the beginning of the second line
    WriteStringToLCD("$$$$$$$$$$$$$$$$");
    // 7 segment stop decreasing
    int con = time;
    while(1)
    sevenseg(con);
    
    
}
void sevenseg(int time)
{
    int seg0,seg1,seg2,seg3;
    if(starttime == 1 && time < 0)
    {
        time = 0;
        programbitti = 1;
    }
    if(starttime == 0)
    {
        seg0 = 10;
        seg1 = 10;
        seg2 = 10;
        seg3 = 10;
    }
    else{
    seg0 = 0;
    seg1 = time/100;
    seg2 = (time - seg1*100)/10;
    seg3 = time %10;}
    PORTJ=sevseg[seg0];RH0=1; //Turn ON display 1 and print 4th digit
    __delay_ms(5);RH0=0;     //Turn OFF display 1 after 5ms delay
    
    PORTJ=sevseg[seg1];RH1=1; //Turn ON display 1 and print 4th digit
    __delay_ms(5);RH1=0;     //Turn OFF display 1 after 5ms delay
    
    PORTJ=sevseg[seg2];RH2=1; //Turn ON display 1 and print 4th digit
    __delay_ms(5);RH2=0;     //Turn OFF display 1 after 5ms delay
    
    PORTJ=sevseg[seg3];RH3=1; //Turn ON display 1 and print 4th digit
   __delay_ms(5);RH3=0;     //Turn OFF display 1 after 5ms delay
}
void initvalues()
{
    karemi = 1;
    adctoggle = 0;
    toggle = 0;
    cursor = 0x8B; // bir rakam yaz?ld???nda bunu artt?r
    counter = 0;
    adcDone = 0;
    setbitti = 0;
    enterbitti = 0;
    blinknewpin = 0;
    attempts = 2;
    trytime = 0;
    sevsegon = 0;
    time = 120;
    countertmr1 = 0;
    starttime = 0;
    
}
void mydelay()
{
    int trv = 135;
    while(--trv)
        sevenseg(time);
}
void _mydelay()
{
    int trv = 113;
    while(--trv)
        sevenseg(time);
}