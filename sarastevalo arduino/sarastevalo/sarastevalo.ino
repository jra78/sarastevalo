/*

Dawn simulator
ds1307
Omron A7D thumbwheel
74165

*/
//#define F_CPU 16000000  //not needed in arduino

#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>

//needed for RTC library
 tmElements_t tm;
 
  // output
const int PWMpin = 6 ;//pd6


//74165 pins
const int DATApin = 8;
const int CLKpin = 9 ;
const int LATCHpin = 10 ;


// rotary encder pins

const int UPpin = 2 ;
const int DOWNpin =3  ;
//Manual pwm control

volatile int pwm =0;

//encoder stop or speedup
int estop = 0;
int espeedup = 0;

void setup() {
  
//start serial (debug)  
Serial.begin(9600);

//Pin modes right
pinMode (DATApin , INPUT);
pinMode (LATCHpin,OUTPUT);
pinMode (CLKpin, OUTPUT);
pinMode (PWMpin , OUTPUT);
pinMode (UPpin , INPUT);
pinMode (DOWNpin , INPUT);

//interrupts
    attachInterrupt(0, encoder, FALLING);
}

bool conf()
//config, any not valid wake up/dawn time will put clock in set time mode. Set time with thumbwheels and accept with encoder twist.
{
  unsigned int hours;
  unsigned int minutes;
  detachInterrupt(0); //we dont want interrupts here
  delay (5);
  
  while (digitalRead (3)== HIGH)
  {
    digitalWrite (PWMpin, HIGH);
  } //stay this line until time is set, accept set time with knob twist
// reuse alarm functions to read time to set to RTC (only hours and minutes. date, month and year cant cange at the moment
   unsigned int alarm = readsetalarmbin();
  hours = alarmhours (alarm);
  minutes =alarmminutes (alarm);    
  
  
  tm.Hour = hours;
  tm.Minute = minutes;
  tm.Second = 0;
  tm.Day = 01;
  tm.Month = 01;
  tm.Year = 2015;
  
  
   
    attachInterrupt(0, encoder, FALLING); //reactivate interrupts
    
    if (RTC.write(tm)) 
    {
      digitalWrite (PWMpin, HIGH);
      delay (50);
            digitalWrite (PWMpin, LOW);
            delay (50);

                 digitalWrite (PWMpin, HIGH);
delay(200);
      digitalWrite (PWMpin, LOW);
delay(200);

      return true;
    }
    else
    {
            digitalWrite (PWMpin, HIGH);
delay(50);
      digitalWrite (PWMpin, LOW);

      return false;
    }
}

void encoder()
{
  //delayMicroseconds (3000);// just a little debounce
 if (digitalRead (3)==LOW)
 {
   if (pwm>254)
   {
     if (estop==0)
     {
       pwm =0; 
     }  
   }
   else 
   {
     pwm++ ;
   }
 
 }
  else if (digitalRead (3)==HIGH)
 {
   if (pwm<1)
   {
     if (estop==0)
     {
       pwm =255; 
     }  
   }
   else
  pwm-- ;
 }
  analogWrite (PWMpin, pwm);
}



unsigned int readsetalarmbin(){
  // read data from switches
  
   byte pins =0;
   unsigned int alarm =0;
  //read data from switches to 74165
    digitalWrite(LATCHpin, LOW);
    delayMicroseconds(5);
    digitalWrite(LATCHpin, HIGH);
    
    //read data from 74165 to arduino
    for (int i=0; i < 16; i++)
   {
      pins = digitalRead (DATApin);  
        digitalWrite(CLKpin, HIGH);
        delayMicroseconds(5);
        digitalWrite(CLKpin, LOW);
        
        
        alarm |= (pins << (15-i));
        
        

    }
    return alarm;
}

void dawn(float delayed)
// dawn, will increase PWM from 0 to 255 in time set by delayed (minutes)
//will ignore pwm changes if i is smaller than pwm. pwm 255 will be after same delay regardless of satarting pwm and pwm will not go 0 if there is already pwm
{
  float viive=0;
  viive = (((delayed*60)/255)*1000); //delay betweed light changes in milliseconds
  for (int i=0; i<256;i++)
    {
      if (i >= pwm)
      {
        analogWrite (PWMpin, pwm);
      
        if (pwm ==255)
          {
            break;
          }
          else 
          {
            pwm++;
          }
      }
      delay (viive);
  
  }
   
   digitalWrite (PWMpin, HIGH);

}



byte turn (byte turn)
{
  switch (turn)
  {
   case B0001:
   turn = B1000 ;
   break;
  
      case B0010:
   turn = B0100 ;
   break;
  
      case B0011:
   turn = B1100 ;
   break;
  
      case B0100:
   turn = B0010 ;
   break;
  
      case B0101:
   turn = B1010 ;
   break;
  
      case B0110:
   turn = B0110 ;
   break;
  
      case B0111:
   turn = B1110 ;
   break;
  
      case B1000:
   turn = B0001 ;
   break;
  
      case B1001:
   turn = B1001 ;
   break;
  
      case B1010:
   turn = B0101 ;
   break;
  
      case B1011:
   turn = B1101 ;
   break;
  
      case B1100:
   turn = B0011 ;
   break;
  
      case B1101:
   turn = B1011 ;
   break;
  
      case B1110:
   turn = B0111 ;
   break;
  
      case B1111:
   turn = B1111 ;
   break;
  
   
 
  }
  
  return (turn);
}


unsigned int alarmminutes (unsigned int alarm)
//return hours in unsig.integer
{   
        byte tunnit1 = alarm & B00001111;         
        byte tunnit2 = (alarm >>4 & B00001111);
        // because of an error in schematic, turn bits
        tunnit1 = turn(tunnit1);
        tunnit2 = turn(tunnit2);
        
        byte tunnit = (10 * tunnit1 + tunnit2);
        
        return (tunnit);

}


unsigned int alarmhours (unsigned int alarm)
//return minutes in unsig.integer
{   
        byte minutes1 = alarm >>8& B00001111;         
        byte minutes2 = (alarm >>12 & B00001111);
        // because of an error in schematic, turn bits

        minutes1 = turn(minutes1);
        minutes2 = turn(minutes2);
        
        
        byte minuutit = (10 * minutes1 + minutes2);
        
        return (minuutit);

}

void loop() {
 /* / debug
 
 
 conf();
  RTC.read(tm);
 
 
  unsigned int alarm = readsetalarmbin();
  unsigned int hours = alarmhours (alarm);
  unsigned int minutes =alarmminutes (alarm);
  
  
  for (int i=0;i <tm.Hour;i++)
  {
    digitalWrite (PWMpin, HIGH);
    delay (50);
    digitalWrite (PWMpin, LOW);
    delay (50);
  
  }
  
  delay (200);
 //debug */
  
  
  
  
  
  
  
  
  
 
  //read RTC
   tmElements_t tm;
   
   RTC.read(tm);
   
  
// Get alarm time and transfer it to hours and minutes variables   
  unsigned int alarm = readsetalarmbin();
  unsigned int hours = alarmhours (alarm);
  unsigned int minutes =alarmminutes (alarm);
  
//compare RTC-time and alarm time  
  
  if ((tm.Hour == hours) && (tm.Minute) == minutes)
  {
  dawn (30);// wake up!!
  }

//enter config mode

if (hours>23 || minutes>59)

{
  int oldhours=hours;
  int oldminutes=minutes;
  delay (250);
  //debounce
  alarm = readsetalarmbin();
  hours = alarmhours (alarm);
  minutes =alarmminutes (alarm);
  
  if (hours==oldhours && minutes==oldminutes)
  { 
    conf();
  } 

  
//prevent encoder to move too fast drom pwm 0 to255 or 255 to 0

if (pwm == 0 || pwm ==255)
  {
    estop = 1;
    delay (300);
    estop =0;
    delay (700);
  }
}






/* debug-->  
  Serial.print ("alarm time: ");
  Serial.print (hours);
  Serial.print (" ");
  Serial.println (minutes);
  Serial.println (" ");
  delay (500);
  
  Serial.print ("RTC time: ");
  Serial.print(tm.Hour);
  Serial.print (":");
  Serial.println (tm.Minute);
 // <--debug */
  
  }
