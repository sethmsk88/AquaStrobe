//jolliFactory - JF Time Fountain V1.0
//Please visit our instructable at https://www.instructables.com/id/JF-Time-Fountain/ for more detail


#include <PWM.h>                  // PWM Frequency library available at https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads

//#define DEBUG
//uncomment to check serial monitor and see LED heartbeat
//Tactile Switch needs to be pressed longer in debug mode to change mode

#define BASE_FREQ 45
#define MIN_FREQUENCY_OFFSET -8.0
#define MAX_FREQUENCY_OFFSET 8.0
#define MIN_BRIGHTNESS 2          
#define MAX_BRIGHTNESS 10.0       
#define NumofModes 4              // Number of Operational modes

const byte SValve = 3;            // pin for Solenoid Valve
const byte Sensor = 4;            // pin for photoelectric sensor
const byte Water_pump = 5;        // pin for Water pump control
const byte ButtonSW = 6;          // pin for mode selection button
const byte LED_strip = 10;        // pin for LED strip control
const byte LED = 13;              // pin for on-board LED

boolean led_on = true;
boolean mode_changed = true;
boolean increaseFrequency_led = true;

byte mode = 1; //toggle with button SW
//mode 1 = normal mode (power on)
//mode 2 = demo mode
//mode 3 = levitation mode
//mode 4 = light off mode

byte buttonState = 0;            
byte lastButtonState = 0;        
byte val_Sensor = 0;

float frequency_offset = 0.1;
float duty_sValve = 18;
float frequency_sValve = BASE_FREQ;  
float duty_led = 7;  
float frequency_led = frequency_sValve+frequency_offset; 
float totalcount = 0;

int lastBrightnessValue = 0;
int pumpIntensity = 0;



//**********************************************************************************************************************************************************
void setup()
{
  Serial.begin(9600);

  pinMode(LED, OUTPUT);             // Heart Beat LED
  pinMode(ButtonSW, INPUT_PULLUP);  // Mode button    
  pinMode(Water_pump, OUTPUT);      // Water_pump
  pinMode(Sensor, INPUT);           // Sensor
      
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe();

  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(LED_strip, frequency_led);
  
  bool success2 = SetPinFrequencySafe(SValve, frequency_sValve);
  
  //if the pin frequency was set successfully, turn LED on
  if(success and success2) 
    digitalWrite(LED, HIGH);        
}



//**********************************************************************************************************************************************************
void loop()
{      
  if (mode_changed == true)
  {
    if (mode == 1)  //normal mode
    {   
      frequency_sValve = BASE_FREQ;
      sValve_on();    
      led_on = true;
    }
    else if (mode == 2)  // demo mode
    {
      //frequency_led shall change in main loop
    }
    else if (mode == 3)  // levitation mode
    {
      frequency_led = BASE_FREQ;
    }
    else if (mode == 4)  // Light off but water pump is still on
    { 
      led_on = false;
    }

    mode_changed = false; 
  }

  
// *******************************************************************************  
  pumpIntensity = map(analogRead(A2), 0, 1023, 100, 255);
  waterPump_on();


// *******************************************************************************    
  val_Sensor = digitalRead(Sensor);     // read the sensor pin

  if (mode == 1)
  {        
    if (val_Sensor == 1)  // sensor unblocked
    {
      frequency_offset = -(MAX_FREQUENCY_OFFSET-MIN_FREQUENCY_OFFSET)/1023L*analogRead(A1)+MAX_FREQUENCY_OFFSET;
      totalcount = 0;
    }
    else //sensor blocked
    {
      totalcount = totalcount + 0.001;

      frequency_offset = -(MAX_FREQUENCY_OFFSET-MIN_FREQUENCY_OFFSET)/1023L*analogRead(A1)+MAX_FREQUENCY_OFFSET + totalcount;

      if (frequency_offset < MIN_FREQUENCY_OFFSET)
        frequency_offset = MIN_FREQUENCY_OFFSET;

      if (frequency_offset > MAX_FREQUENCY_OFFSET)
        frequency_offset = MAX_FREQUENCY_OFFSET;
    }
  }
  else  
    frequency_offset = -(MAX_FREQUENCY_OFFSET-MIN_FREQUENCY_OFFSET)/1023L*analogRead(A1)+MAX_FREQUENCY_OFFSET;


// *******************************************************************************  
  if (led_on == true)
  {    
    duty_led = -(MAX_BRIGHTNESS-MIN_BRIGHTNESS)/1023L*analogRead(A0)+MAX_BRIGHTNESS;
    
    if (mode == 2)   // demo mode
    {
      if (frequency_led <= (BASE_FREQ + MIN_FREQUENCY_OFFSET) + 2)
      {
        increaseFrequency_led = true;
        totalcount = 0;
      }
      else if (frequency_led >= (BASE_FREQ + MAX_FREQUENCY_OFFSET) - 2)
      {
        increaseFrequency_led = false;
        totalcount = 0; 
      }
      else if ((frequency_led > (BASE_FREQ - 1)) and (frequency_led < (BASE_FREQ + 1)))  // Slow down when frequency of LED is near to Base Frequency - water droplets about to levitate
      {
        delay(360);
      }

      totalcount = totalcount + 0.001;

      if (increaseFrequency_led == true)
        frequency_led += totalcount;
      else
        frequency_led -= totalcount;

      delay(120);   
    }      
    else if (mode == 1)
      frequency_led = frequency_sValve+frequency_offset;

    
    SetPinFrequencySafe(LED_strip, frequency_led);  
           
                        
    if (lastBrightnessValue < round(duty_led*255/100))  //previously dimmer - gradually bright it
    {
      for (int i=lastBrightnessValue; i<round(duty_led*255/100); i++)
      {
        pwmWrite(LED_strip, i);
        delay(30);
      }
    } 
    else if (lastBrightnessValue > round(duty_led*255/100)) //previously brighter - gradually dim it
    {
      for (int i=lastBrightnessValue; i>round(duty_led*255/100); i--)
      {
        pwmWrite(LED_strip, i);
        delay(30);      
      }
    }
    else  //no change in brightness
      pwmWrite(LED_strip, round(duty_led*255/100));   

    lastBrightnessValue = round(duty_led*255/100);     
  }
  else
  {
    //gradually dim off
    for (int i=round(duty_led*255/100); i>0; i--)
    {
      pwmWrite(LED_strip, i);
      delay(30);
    }
      
    duty_led = 0;      
    pwmWrite(LED_strip, 0);
    lastBrightnessValue = 0;
  } 
 
 
// *******************************************************************************  
  #ifdef DEBUG
    //Heatbeat on-board LED
    digitalWrite(LED, HIGH);
    delay(300);
    digitalWrite(LED, LOW);
    delay(300); 
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(1200);     
    
    
    //serial print current parameters    
    Serial.print("Mode: "); 
    Serial.print(mode);
    Serial.print("  Frequency Offset: "); 
    Serial.print(frequency_offset);
    Serial.print("  Force: ");
    Serial.print(duty_sValve);
    Serial.print("  Freq Valve: ");
    Serial.print(frequency_sValve);
    Serial.print("  Freq LED: ");    
    Serial.print(frequency_led);       
    Serial.print("  Brightness: ");
    Serial.println(duty_led);    
  #endif

    
// *******************************************************************************  
  // read the button SW
  buttonState = digitalRead(ButtonSW);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) 
  {
    // if the state has changed, increment the mode count
    if (buttonState == LOW) 
    {    
      mode++;

      if (mode > NumofModes)
        mode = 1; //rotary menu
      
      mode_changed = true ;      
    }

    // delay a little bit for button debouncing
    delay(50);
  }

  lastButtonState = buttonState;
}



//**********************************************************************************************************************************************************
void sValve_on() 
{
  pwmWrite(SValve, 115);
}



//**********************************************************************************************************************************************************
void sValve_off() 
{
  pwmWrite(SValve, 0);  
}



//**********************************************************************************************************************************************************
void waterPump_on() 
{
  analogWrite(Water_pump,pumpIntensity);
}



//**********************************************************************************************************************************************************
void waterPump_off() 
{
  analogWrite(Water_pump,0);
}
