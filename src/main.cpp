#include <Arduino.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4

// define analog pin connected to potentiometer
const int potPin = A3;

// define digital PWM pin to MOSFET gate
const int PWM_pin = 6;

// define address and config for OLED display
Adafruit_SSD1306 display(OLED_RESET);
const byte SSD1306_ADDR_1 = 0x3C;

int thermistorPin = A2;
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

float set_temperature = 0;
float temperature_read = 0.0;
float PID_error = 0;
float previous_error = 0;
float elapsedTime, Time, timePrev;
float PID_value = 0;

// PID constants
//////////////////////////////////////////////////////////
int kp = 20;   int ki = 0.3;   int kd = 2;
//////////////////////////////////////////////////////////

int PID_p = 0;    int PID_i = 0;    int PID_d = 0;
float last_kp = 0;
float last_ki = 0;
float last_kd = 0;

void setup() {
  // initialize serial communication
  // Serial.begin(9600);

  // initialize display
  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_ADDR_1);
  display.clearDisplay();

  pinMode(PWM_pin, OUTPUT);
}

void loop() {
  // Clear display per iteration then configure text size, cursor, and color
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  // Read the analog value from the potentiometer
  set_temperature = analogRead(potPin) / 4;

  /* --UNCOMMENT FOR DEBUGGING POTENTIOMETER-- 
  // Write analog value to MOSFET
  //digitalWrite(6,HIGH);
  analogWrite(PWM_pin,potValue);

  // Print the potentiometer value to the serial monitor
  Serial.print("Potentiometer value: ");
  Serial.println(set_temperature);

  // Print the potentiometer value to OLED display
  display.print("Potentiometer value: "); display.print(set_temperature);
  display.println();
  */ 

  Vo = analogRead(thermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  T = (T * 9.0)/ 5.0 + 32.0; 
  temperature_read = T + 90;

  /* --UNCOMMENT FOR DEBUGGING TEMPERATURE READING-- 
  // Print the temperature value to the serial monitor
  Serial.print("Temperature: "); 
  Serial.print(temperature_read);
  Serial.println(" F"); 
  */

  // calculate the error between set temp and real temp
  PID_error = set_temperature - temperature_read;
  // calculate P value
  PID_p = kp * PID_error;
  // calculate I value
  PID_i = PID_i + (ki * PID_error);
  
  timePrev = Time; // the previous time is stored before the actual time read
  Time = millis(); // actual time read
  elapsedTime = (Time - timePrev) / 1000; 
  // calculate D value
  PID_d = kd*((PID_error - previous_error)/elapsedTime);
  // total PID value is the sum of P + I + D
  PID_value = PID_p + PID_i + PID_d;

  // define PWM range between 0 and 255
  if(PID_value < 0)
  {    PID_value = 0;    }
  if(PID_value > 255)  
  {    PID_value = 255;  }
  // write the PWM signal to the mosfet
  analogWrite(PWM_pin,PID_value);
  // store the previous error for next loop
  previous_error = PID_error;

  display.print("PID TEMP control");
  display.println();
  display.print("Set: "); display.print(set_temperature); 
  display.println();
  display.print("Read: "); display.print(temperature_read);
  display.println();
  display.print("PiD: "); display.print(PID_value);
  display.print(" Time: "); display.print(Time);
  display.println();
  display.display();

  // add a small delay for stability
  delay(100);
}