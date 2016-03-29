/* 
 * TIME BASED VOLUME CONTROL FOR MUSIC CONSTRUCTION MACHINE [runs on Arduino UNO]
 * 
 * Niklas Roy / 2016 / www.niklasroy.com
 * 
 * Published under the beer-ware license 
 * 
 * Purpose of this program is to control a mixer poti with a fader, based on the current daytime, which is provided by an external realtime clock.
 * The idea is to have louder volume at day, more silent volume in the evening and probably no sound at night.
 * For that purpose, three servo positions and three trigger times (day/evening/night) can be stored in the EEPROM of the microcontroller.
 * The program automatically adjusts the volume knob to the desired position, when a trigger time is reached.
 *
 * DS1307 i²c realtime clock module connected to SDA & SCL 
 * volume servo on D6
 * LCD Enable on pin 11
 * LCD RS on pin 12
 * LCD D4 on pin 5
 * LCD D5 on pin 4
 * LCD D6 on pin 3
 * LCD D7 on pin 2 
 * poti connected to A0
 * button 1 connected to A2 (with 10K pulldown resistor)
 * piezo speaker connected to A1
 * 
 */
 
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"                     // DS1307 library
#include <Servo.h>                      // Servo library
#include <LiquidCrystal.h>              // LCD library

#define POTI analogRead(A0)
#define BUTTON1 digitalRead(A2)
#define SPEAKER A1

#define DAY_HOUR 0                      // EEPROM memory locations
#define DAY_MINUTE 2  
#define DAY_VOLUME 4  
#define EVENING_HOUR 6  
#define EVENING_MINUTE 8  
#define EVENING_VOLUME 10 
#define NIGHT_HOUR 12 
#define NIGHT_MINUTE 14 
#define NIGHT_VOLUME 16 

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // initialize LCD library 
Servo myservo;                          // create a Servo object
RTC_DS1307 rtc;                         // create a RealTimeClock object

int p_state=0;
int state=0;
int volume;
int rtc_minute;
int rtc_hour;
int day_hour;
int day_minute;
int day_volume;
int evening_hour;
int evening_minute;
int evening_volume;
int night_hour;
int night_minute;
int night_volume;
int match_time;
int p_desired_servo_pos;
int desired_servo_pos;
int servo_pos;
int detach_timeout;


void setup() {
  pinMode(SPEAKER,OUTPUT);
  beep();
  lcd.begin(16, 2);   // configure to 16x2 LCD
  lcd.print("time based      "); 
  lcd.setCursor(0, 1);
  lcd.print("volume control  ");
  
  day_hour       = EEPROM.read(DAY_HOUR);       //read volume and trigger time values from eeprom
  day_minute     = EEPROM.read(DAY_MINUTE);
  day_volume     = EEPROM.read(DAY_VOLUME);
  evening_hour   = EEPROM.read(EVENING_HOUR);
  evening_minute = EEPROM.read(EVENING_MINUTE);
  evening_volume = EEPROM.read(EVENING_VOLUME);
  night_hour     = EEPROM.read(NIGHT_HOUR);
  night_minute   = EEPROM.read(NIGHT_MINUTE);
  night_volume   = EEPROM.read(NIGHT_VOLUME);

  rtc.begin();        // start the RTC

  
  delay(1000);
  lcd.clear();        // clear screen
  lcd.setCursor(0,0);
  beep();beep2();
}

void loop() {
  p_state = state;
  state = map(POTI,0,1023,6,1);          // change state based on poti position
  if(p_state!=state){lcd.clear();tic();} // make a little sound and clear LCD if state changes
  
  switch (state){                        // jump to different states
    case 1:
      display_time_and_volume();
    break;
    case 2:
      set_volume_day();
    break;
    case 3:
      set_volume_evening();
    break;
    case 4:
      set_volume_night();
    break;
    case 5:
      set_trigger_times();
    break;
    case 6:
      set_time();
    break;
  }

  set_volume_to_current_time();
  smooth_servo(volume,0);          // set servo
  delay(15);
  
}

//---------------------------------------------------------- display time and volume
void display_time_and_volume(){
  DateTime now = rtc.now();       // read time from RTC chip
  lcd.setCursor(0, 0); 
  lcd.print("  ");    
  if(now.hour()<10)lcd.print("0");    
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  if(now.minute()<10)lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if(now.second()<10)lcd.print("0");
  lcd.print(now.second());
  if (match_time==1){lcd.print(" (d)");}
  if (match_time==2){lcd.print(" (e)");}
  if (match_time==3){lcd.print(" (n)");}
  lcd.setCursor(0, 1);
  lcd.print("  volume: ");
  lcd.print(volume);
  lcd.print("  ");
}
//---------------------------------------------------------- set time
void set_time(){
  lcd.setCursor(0, 0);
  lcd.print("set current time");
  
  if (BUTTON1){
    lcd.clear();
    delay(100);
    while(BUTTON1){};
    delay(100);
    
    while(!BUTTON1){    // read hour input
        lcd.setCursor(0, 0);
        lcd.print("hour:  ");
        rtc_hour=map(POTI,1023,0,0,23);
        if (rtc_hour<10)lcd.print("0");
        lcd.print(rtc_hour);
      }
      
    lcd.clear();
    delay(100);
    while(BUTTON1){};
    delay(100);
    
    while(!BUTTON1){    // read minute input
        lcd.setCursor(0, 0);
        lcd.print("minute: ");
        rtc_minute=map(POTI,1023,0,0,59);
        if (rtc_minute<10)lcd.print("0");
        lcd.print(rtc_minute);
      }
      
    lcd.clear();
    delay(100);
    while(BUTTON1){};
    delay(100);
    
    rtc.adjust(DateTime(2016,1,1,rtc_hour,rtc_minute,0)); //set time:(year,month,day,hour,minute,seconds)
    }
  }

//---------------------------------------------------------- set volume day
void set_volume_day(){
  lcd.setCursor(0, 0);
  lcd.print("volume day:     ");
  lcd.setCursor(0, 1);
  if(day_volume<10) lcd.print("0");
  if(day_volume<100)lcd.print("0");
  lcd.print(day_volume);
    if (BUTTON1){     // button pressed? go on with editing volume
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);

    while(!BUTTON1){  //read volume input
      lcd.setCursor(0, 0);
      lcd.print("new day volume: ");
      lcd.setCursor(0, 1);
      day_volume=map(POTI,1023,0,0,100);
      if (day_volume<100)lcd.print("0");
      if (day_volume<10)lcd.print("0");
      lcd.print(day_volume);
      smooth_servo(day_volume,1);          // set servo
      delay(15);
    }
 
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);
    
    EEPROM.write(DAY_VOLUME, day_volume); //save day volume
  }
}

//---------------------------------------------------------- set volume evening
void set_volume_evening(){
  lcd.setCursor(0, 0);
  lcd.print("volume evening: ");
  lcd.setCursor(0, 1);
  if(evening_volume<10) lcd.print("0");
  if(evening_volume<100)lcd.print("0");
  lcd.print(evening_volume);
    if (BUTTON1){     // button pressed? go on with editing volume
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);

    while(!BUTTON1){  //read volume input
      lcd.setCursor(0, 0);
      lcd.print("new evening vol:");
      lcd.setCursor(0, 1);
      evening_volume=map(POTI,1023,0,0,100);
      if (evening_volume<100)lcd.print("0");
      if (evening_volume<10)lcd.print("0");
      lcd.print(evening_volume);
      smooth_servo(evening_volume,1);          // set servo
      delay(15);
    }
 
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);
    
    EEPROM.write(EVENING_VOLUME, evening_volume); //save evening volume
  }
}
//---------------------------------------------------------- set volume night
void set_volume_night(){
  lcd.setCursor(0, 0);
  lcd.print("volume night: ");
  lcd.setCursor(0, 1);
  if(night_volume<10) lcd.print("0");
  if(night_volume<100)lcd.print("0");
  lcd.print(night_volume);
    if (BUTTON1){     // button pressed? go on with editing volume
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);

    while(!BUTTON1){  //read volume input
      lcd.setCursor(0, 0);
      lcd.print("new night vol:");
      lcd.setCursor(0, 1);
      night_volume=map(POTI,1023,0,0,100);
      if (night_volume<100)lcd.print("0");
      if (night_volume<10)lcd.print("0");
      lcd.print(night_volume);
      smooth_servo(night_volume,1);          // set servo
      delay(15);
    }
 
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);
    
    EEPROM.write(NIGHT_VOLUME, night_volume); //save night volume
  }
}
//---------------------------------------------------------- set trigger times
void set_trigger_times(){
  lcd.setCursor(0, 0);
  lcd.print("trigger times   ");
  switch ((millis()/3000)%3){
    case 0:
      lcd.setCursor(0, 1);
      lcd.print("day: ");
      if (day_hour<10)lcd.print("0");
      lcd.print(day_hour);
      lcd.print(":");
      if (day_minute<10)lcd.print("0");
      lcd.print(day_minute);
      lcd.print("    ");
    break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("evening: ");
      if (evening_hour<10)lcd.print("0");
      lcd.print(evening_hour);
      lcd.print(":");
      if (evening_minute<10)lcd.print("0");
      lcd.print(evening_minute);
    break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print("night: ");
      if (night_hour<10)lcd.print("0");
      lcd.print(night_hour);
      lcd.print(":");
      if (night_minute<10)lcd.print("0");
      lcd.print(night_minute);
      lcd.print("  ");
    break;
  }

  if (BUTTON1){     // button pressed? go on with editing trigger times
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);

    while(!BUTTON1){  //read day hour input
      lcd.setCursor(0, 0);
      lcd.print("new day hour:");
      lcd.setCursor(0, 1);
      day_hour=map(POTI,1023,0,0,23);
      if (day_hour<10)lcd.print("0");
      lcd.print(day_hour);
      lcd.print(":");
      if (day_minute<10)lcd.print("0");
      lcd.print(day_minute);
    }

    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);

    while(!BUTTON1){  //read day minute input
      lcd.setCursor(0, 0);
      lcd.print("new day minute:");
      lcd.setCursor(0, 1);
      day_minute=map(POTI,1023,0,0,59);
      if (day_hour<10)lcd.print("0");
      lcd.print(day_hour);
      lcd.print(":");
      if (day_minute<10)lcd.print("0");
      lcd.print(day_minute);
    }
    
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);
    
    while(!BUTTON1){  //read evening hour input
      lcd.setCursor(0, 0);
      lcd.print("new evening hr.:");
      lcd.setCursor(0, 1);
      evening_hour=map(POTI,1023,0,0,23);
      if (evening_hour<10)lcd.print("0");
      lcd.print(evening_hour);
      lcd.print(":");
      if (evening_minute<10)lcd.print("0");
      lcd.print(evening_minute);
    }

    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);

    while(!BUTTON1){  //read evening minute input
      lcd.setCursor(0, 0);
      lcd.print("new evening min:");
      lcd.setCursor(0, 1);
      evening_minute=map(POTI,1023,0,0,59);
      if (evening_hour<10)lcd.print("0");
      lcd.print(evening_hour);
      lcd.print(":");
      if (evening_minute<10)lcd.print("0");
      lcd.print(evening_minute);
    }
    
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);
    
    while(!BUTTON1){  //read night hour input
      lcd.setCursor(0, 0);
      lcd.print("new night hour:");
      lcd.setCursor(0, 1);
      night_hour=map(POTI,1023,0,0,23);
      if (night_hour<10)lcd.print("0");
      lcd.print(night_hour);
      lcd.print(":");
      if (night_minute<10)lcd.print("0");
      lcd.print(night_minute);
    }

    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);

    while(!BUTTON1){  //read night minute input
      lcd.setCursor(0, 0);
      lcd.print("new night min.:");
      lcd.setCursor(0, 1);
      night_minute=map(POTI,1023,0,0,59);
      if (night_hour<10)lcd.print("0");
      lcd.print(night_hour);
      lcd.print(":");
      if (night_minute<10)lcd.print("0");
      lcd.print(night_minute);
    }
    
    lcd.clear();
    beep();
    delay(100);
    while(BUTTON1){}; //debounce
    delay(100);

    EEPROM.write(DAY_HOUR,day_hour); //save trigger times
    EEPROM.write(DAY_MINUTE,day_minute);
    EEPROM.write(EVENING_HOUR,evening_hour); 
    EEPROM.write(EVENING_MINUTE,evening_minute);
    EEPROM.write(NIGHT_HOUR,night_hour); 
    EEPROM.write(NIGHT_MINUTE,night_minute);
    
  }   
}

//---------------------------------------------------------- beep
void beep(){
  for (int i=0;i<50;i++){
  digitalWrite(SPEAKER,LOW);
  delay(1);
  digitalWrite(SPEAKER,HIGH);
  delay(1);
  }
}
//---------------------------------------------------------- beep
void beep2(){
  for (int i=0;i<50;i++){
  digitalWrite(SPEAKER,LOW);
  delayMicroseconds(500);
  digitalWrite(SPEAKER,HIGH);
  delayMicroseconds(500);
  }
}
//---------------------------------------------------------- tic
void tic(){
  for (int i=0;i<10;i++){
  digitalWrite(SPEAKER,LOW);
  delayMicroseconds(200);
  digitalWrite(SPEAKER,HIGH);
  delayMicroseconds(200);
  }
}
//---------------------------------------------------------- set volume to current time
void set_volume_to_current_time(){
  DateTime now = rtc.now();       // read time from RTC chip
                                  // set volume according tocurrent time (cycle through the past max. 24 hrs in order to find a matching trigger time)
  int scan_minute=now.minute();
  int scan_hour=now.hour();
  match_time=0;

  while (!match_time){
    if (scan_hour==day_hour && scan_minute==day_minute)        {volume=day_volume;     match_time=1;}  
    if (scan_hour==evening_hour && scan_minute==evening_minute){volume=evening_volume; match_time=2;}  
    if (scan_hour==night_hour && scan_minute==night_minute)    {volume=night_volume;   match_time=3;}
    if (scan_minute>0){
      scan_minute--;
    }else{
      scan_minute=59;
      if (scan_hour>0){
        scan_hour--;
      }else{
        scan_hour=23;
      }
    }  
  }
}
//---------------------------------------------------------- smooth servo output / moves slowly to new servo position and attaches/detaches servo automatically
void smooth_servo(int destination, int direct_access){ //destination ranging from 0-100
  desired_servo_pos=destination;
  
  if (desired_servo_pos != p_desired_servo_pos){
    myservo.attach(6);  // attaches the servo on pin 6 to the servo object
    p_desired_servo_pos=desired_servo_pos;
    detach_timeout=0;
  }

  if (desired_servo_pos != servo_pos){
    if (desired_servo_pos>servo_pos)servo_pos++;
    if (desired_servo_pos<servo_pos)servo_pos--;
    if (direct_access==1)servo_pos=desired_servo_pos;
    if (desired_servo_pos==servo_pos)detach_timeout++;
    myservo.write(map(servo_pos,0,100,170,28));
  }

  if (desired_servo_pos==servo_pos && detach_timeout<51){
    detach_timeout++;
  }

  if (detach_timeout == 50)myservo.detach();
}
