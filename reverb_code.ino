#include <Wire.h>

#define DC_VAL 2048
#define AMPLIT 2040
#define RDH 0x10 // RDY high (use or)
#define RDL 0xc0; // RDY low (use and)
#define DELAY_SIZE 1256


unsigned long currT, prevT = 0;
byte buff;
byte delayBank[DELAY_SIZE];
int delayBankI=0;
int mySignal=0, delayWet=0;
byte adcVal = 0;
float mix = 0.4;





void setup()
{
  Wire.begin(); 
  TWBR = 12; //400kHz clock for I2C
  DDRD = DDRD & B00000011;  // set pins 2 -> 7 as inputs (interrupt + 5 pins from ADC)
  DDRB = DDRB & B11000000; // set the pins 8 -> 10 as inputs (the rest 3 pins from ADC)  
  DDRB = DDRB | B00010000; // set the WR/RDY pin 12 as output

  PORTB |= RDH; // WR/RDY starts as 1 by default
  
  attachInterrupt(digitalPinToInterrupt(2), getADC, FALLING); // when INT falls get the ADC value
  
}

void getADC() // the interrupt that the !INT pin of the ADC gives to the Arduino
{
  adcVal = (PIND >> 3) | (PINB << 5); // get ADC value
  
  PORTB |= RDH; //mark the WR/RDY high (see transition diagram in ADC datasheet) 
  
}

void loop()
{
 currT = micros();

 
 
 if(currT-prevT >= 40)
 {
  
  prevT = currT;
  
  
  //READ START
    
    
    buff = adcVal; // read value 
    PORTB &= RDL; 
    delayMicroseconds(0.6); //twr from transition graph and ADC datasheet 
    PORTB |= RDH; // push the WR/RDY for another conversion
    
  //READ STOP

   

  // PROCESSING START

    delayWet = buff + ((delayBank[delayBankI]-127)*mix); // the delayed feedback loop

    //this is because delayWet is going to be converted into byte so I don't want to do overflows
    if(delayWet<0)
    {
      delayWet=0;
    }
    if(delayWet>=255)
    {
      delayWet = 255;
    }

   delayBank[delayBankI] = (byte) delayWet ; // get the delay bank from the output
   
  //PROCESSING STOP


    //SCRIERE START
    
 
  mySignal =  ((delayBank[(delayBankI+1)%DELAY_SIZE]-127)*40)+2048; // the signal is the raw delay bank and we need to
  //convert it from 8 bit to 12 bit with 2048 being the middle
  //I don't use interpolation because it is pretty complicated so the signal is going to be stairy
 
    
  
  Wire.beginTransmission(0x63); //for the physical one because for some reason my unit has the address 0x63 not 60 or 61 (which is usual)
  Wire.write(0x40); //the write code
  //write the 12 bits
  Wire.write((byte)(mySignal >> 4));
  Wire.write((byte)(mySignal << 4));
  Wire.endTransmission();
  
  //SCRIERE STOP
  

  delayBankI = (delayBankI+1)%DELAY_SIZE;


  
 }


  
}
