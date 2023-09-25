#include <Thread.h>
#include <StaticThreadController.h>
#include <ThreadController.h>
#include "InfraredThread.h"
#include "DistanceThread.h"

// Pines del ultrasonido
#define TRIG_PIN 13  
#define ECHO_PIN 12 

// Pines del sensor infrarrojo
#define PIN_ITR20001_LEFT   A2
#define PIN_ITR20001_MIDDLE A1
#define PIN_ITR20001_RIGHT  A0

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

#define MAX_VEL_R 105
#define MAX_VEL_L 105

int counter_lostline = 0;
bool go_car = false;
bool lostline = false;

ThreadController controller = ThreadController();
DistanceThread* distanceThread = new DistanceThread(TRIG_PIN,ECHO_PIN);
InfraredThread* infraredThread = new InfraredThread(PIN_ITR20001_LEFT,PIN_ITR20001_MIDDLE,PIN_ITR20001_RIGHT);
Thread motorsThread = Thread();

void callback_motors() {

  bool line_center = false;
  bool line_right = false;
  bool line_left = false;
  int vel_r;
  int vel_l;

  if (infraredThread->read_middle >= 500) {
    line_center = true;
    vel_r = MAX_VEL_R;
    vel_l = MAX_VEL_L;

    if (infraredThread->read_right >= 250) {
      vel_r = 45;
      vel_l = 95;
      line_right = true;
    } else {
      line_right = false;
    }

    if (infraredThread->read_left >= 240) {
      line_left = true;
      vel_r = 95;
      vel_l = 45;
    } else {
      line_left = false;    
    }
  } else {
    line_center = false;

    if (infraredThread->read_right >= 250) {
      vel_l = 95;
      vel_r = 0;
      line_right = true;
    } else {
      line_right = false;
    }
    if (infraredThread->read_left >= 240) {
      line_left = true;
      vel_r = 95;
      vel_l = 0;
    } else {
      line_left = false;
    }

    if (!line_center && !line_right && !line_left) {
      counter_lostline++;
      if (counter_lostline >= 10 and !lostline) {
        Serial.println("L"); // Lost line -> L
        counter_lostline = 0;
        lostline = true;
      }
    }

    if ((line_center || line_right || line_left ) && lostline) {
      Serial.println("F"); // Found line -> F
      lostline = false;
    }
  }
  
  analogWrite(PIN_Motor_PWMA, vel_r);
  analogWrite(PIN_Motor_PWMB, vel_l);
}

void setup() {

  pinMode(PIN_Motor_STBY, OUTPUT);

  pinMode(PIN_Motor_AIN_1, INPUT);
  pinMode(PIN_Motor_PWMA, INPUT);
  
  pinMode(PIN_Motor_BIN_1, INPUT);
  pinMode(PIN_Motor_PWMB, INPUT);

  motorsThread.enabled = true;
  motorsThread.setInterval(20);
  motorsThread.onRun(callback_motors);
  digitalWrite(PIN_Motor_STBY, 1);
  digitalWrite(PIN_Motor_AIN_1,1);
  digitalWrite(PIN_Motor_BIN_1,1);
  
  // Para comunicacion con la ESP32
  Serial.begin(9600);

  // Arranque medidor de distancia
  distanceThread->setInterval(50);
  controller.add(distanceThread);

  infraredThread->setInterval(50);
  controller.add(infraredThread);

}

void loop() {

  if (Serial.available()) {

    if (Serial.find("CONNECTED")) {
      go_car = true;      
    }
  }
  if (go_car){
    controller.run();
    if (motorsThread.shouldRun()) {
      motorsThread.run();
    }

    if (distanceThread->detected_obs()) {
      Serial.println("O"); // O -> Obstacle
      motorsThread.enabled = false;
      analogWrite(PIN_Motor_PWMA, 0);
      analogWrite(PIN_Motor_PWMB, 0);
      digitalWrite(PIN_Motor_STBY, 0);
    }
  }
  
}
