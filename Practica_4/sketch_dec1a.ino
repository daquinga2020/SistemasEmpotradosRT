#include <Thread.h>

// Enable/Disable motor control.
//  HIGH: motor control enabled
//  LOW: motor control disabled
#define PIN_Motor_STBY 3

// Group A Motors (Right Side)
// PIN_Motor_AIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_AIN_1 7
// PIN_Motor_PWMA: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMA 5

// Group B Motors (Left Side)
// PIN_Motor_BIN_1: Digital output. HIGH: Forward, LOW: Backward
#define PIN_Motor_BIN_1 8
// PIN_Motor_PWMB: Analog output [0-255]. It provides speed.
#define PIN_Motor_PWMB 6


Thread motorsThread = Thread();

void callback_motors() {
  digitalWrite(PIN_Motor_STBY, HIGH);  
  digitalWrite(PIN_Motor_AIN_1, HIGH); 
  digitalWrite(PIN_Motor_BIN_1, HIGH);

  analogWrite(PIN_Motor_PWMA,0); 
  analogWrite(PIN_Motor_PWMB,0);
}

void setup() {
  
  Serial.begin(9600);

  motorsThread.enabled = true;
  motorsThread.setInterval(250);
  motorsThread.onRun(callback_motors);

  pinMode(PIN_Motor_STBY, OUTPUT);

  pinMode(PIN_Motor_AIN_1, INPUT);
  pinMode(PIN_Motor_PWMA, INPUT);
  
  pinMode(PIN_Motor_BIN_1, INPUT);
  pinMode(PIN_Motor_PWMB, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(motorsThread.shouldRun()){
    motorsThread.run();
  }
}
